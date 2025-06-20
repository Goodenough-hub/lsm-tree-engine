#include "../../include/sst/sst.h"
#include "../../include/const.h"

SSTBuilder::SSTBuilder(size_t block_size, bool with_bloom) : block_size(block_size), block(block_size)
{
    if (with_bloom)
    {
        this->bloom_filter = std::make_shared<BloomFilter>(BLOOM_FILTER_EXPEXTED_SIZE, BLOOM_FILTER_EXPEXTED_ERROR_RATE);
    }
    meta_entries.clear();
    data.clear();
    first_key.clear();
    last_key.clear();
}

void SSTBuilder::add(const std::string &key, const std::string &value, uint64_t tranc_id)
{
    if (first_key.empty())
    {
        first_key = key;
    }

    // 在 布隆过滤器 中添加key
    if (bloom_filter != nullptr)
    {
        bloom_filter->add(key);
    }

    max_tranc_id_ = std::max(max_tranc_id_, tranc_id);
    min_tranc_id_ = std::min(min_tranc_id_, tranc_id);

    bool force_write = last_key == key;
    // 连续出现的相同的key必须位于同一个block

    if (block.add_entry(key, value, tranc_id, force_write))
    {
        last_key = key;
        return;
    }

    finish_block();

    block.add_entry(key, value, tranc_id, force_write);
    first_key = key;
    last_key = key;
}

size_t SSTBuilder::estimated_size() const
{
    return data.size();
}

void SSTBuilder::finish_block()
{
    // 将block编码data中
    auto old_block = std::move(this->block);
    auto encoded_block = old_block.encode();

    // 把block的元数据也写入
    meta_entries.emplace_back(data.size(), first_key, last_key);

    // 计算哈希校验值
    auto block_hash = static_cast<uint32_t>(std::hash<std::string_view>{}(
        std::string_view(reinterpret_cast<const char *>(encoded_block.data()),
                         encoded_block.size())));

    //  预分配空间并添加数据
    data.reserve(data.size() + encoded_block.size() +
                 sizeof(uint32_t)); // uint32_t 是表示的哈希值

    data.insert(data.end(), encoded_block.begin(), encoded_block.end());
    data.resize(data.size() + sizeof(uint32_t));
    memcpy(data.data() + data.size() - sizeof(uint32_t), &block_hash,
           sizeof(uint32_t));
}

std::shared_ptr<SST>
SSTBuilder::build(size_t sst_id, const std::string &path,
                  std::shared_ptr<BlockCache> block_cache)
{
    if (!block.is_empty())
    {
        finish_block();
    }

    // 判断是否有数据
    if (meta_entries.empty())
    {
        throw std::runtime_error("No data to build SST");
    }

    // 编码元数据块
    std::vector<uint8_t> meta_block;
    BlockMeta::encode_meta_to_slice(meta_entries, meta_block);

    // 计算元数据块的偏移量
    uint32_t meta_offset = data.size();

    // 构建文件内容
    // 1. 写入数据块
    std::vector<uint8_t> file_content = std::move(data);

    // 2. 写入元数据块
    file_content.insert(file_content.end(), meta_block.begin(), meta_block.end());

    // 3. 需要写入布隆过滤器
    uint32_t bloom_offset = file_content.size();
    if (this->bloom_filter != nullptr)
    {
        auto bf_data = bloom_filter->encode();
        file_content.insert(file_content.end(), bf_data.begin(), bf_data.end());
    }

    file_content.resize(file_content.size() + sizeof(uint32_t) * 2 + sizeof(uint64_t) * 2);

    // 4. 编码meta section的offset
    memcpy(file_content.data() + file_content.size() - sizeof(uint32_t) * 2 - sizeof(uint64_t) * 2,
           &meta_offset, sizeof(uint32_t));

    // 5. 编码bloom section的offset
    memcpy(file_content.data() + file_content.size() - sizeof(uint32_t) * 2,
           &bloom_offset, sizeof(uint32_t));

    // 6. 记录最大最小事务id信息
    memcpy(file_content.data() + file_content.size() - sizeof(uint64_t) * 2,
           &min_tranc_id_, sizeof(uint64_t));
    memcpy(file_content.data() + file_content.size() - sizeof(uint64_t),
           &max_tranc_id_, sizeof(uint64_t));

    FileObj file = FileObj::create_and_write(path, file_content);

    auto res = std::make_shared<SST>();

    res->sst_id = sst_id;
    res->file = std::move(file);
    res->first_key = meta_entries.front().first_key;
    res->last_key = meta_entries.back().last_key;
    res->meta_entries = std::move(meta_entries);
    res->bloom_filter = this->bloom_filter;
    res->bloom_offset = bloom_offset;
    res->meta_block_offset = meta_offset;
    res->cache = block_cache;

    res->min_tranc_id_ = min_tranc_id_;
    res->max_tranc_id_ = max_tranc_id_;

    return res;
}

SstIterator SST::get(const std::string &key, uint64_t tranc_id)
{
    // 在布隆过滤器判断key是否存在
    if (bloom_filter != nullptr && !bloom_filter->possibly_contains(key))
    {
        return this->end(tranc_id);
    }
    return SstIterator(shared_from_this(), key, tranc_id);
}

size_t SST::num_blocks()
{
    return meta_entries.size();
}

SstIterator SST::begin(uint64_t tranc_id)
{
    return SstIterator(shared_from_this(), tranc_id);
}

SstIterator SST::end(uint64_t tranc_id)
{
    auto res = SstIterator(shared_from_this(), tranc_id);
    res.m_sst = nullptr;
    res.m_block_idx = -1;
    res.cached_value = std::nullopt;
    return res;
}

std::shared_ptr<SST> SST::open(size_t sst_id, FileObj file,
                               std::shared_ptr<BlockCache> cache)
{
    auto sst = std::make_shared<SST>();
    sst->sst_id = sst_id;
    sst->file = std::move(file);
    sst->cache = cache;

    // 读取文件末尾的元数据块
    // 1. 读取偏移量
    size_t file_size = sst->file.size();
    if (file_size < sizeof(uint32_t))
    {
        throw std::runtime_error("File size is too small");
    }

    auto bloom_offset_bytes =
        sst->file.read_to_slice(file_size - sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&sst->bloom_offset, bloom_offset_bytes.data(), sizeof(uint32_t));

    auto meta_offset_bytes = sst->file.read_to_slice(
        file_size - sizeof(uint32_t) * 2, sizeof(uint32_t));
    memcpy(&sst->meta_block_offset, meta_offset_bytes.data(), sizeof(uint32_t));

    // 2. 读取 bloom filter
    if (sst->bloom_offset + 2 * sizeof(uint32_t) < file_size)
    {
        // 布隆过滤器偏移量 + 2*uint32_t 的大小小于文件大小
        // 表示存在布隆过滤器
        uint32_t bloom_size = file_size - sst->bloom_offset - sizeof(uint32_t) * 2;
        auto bloom_bytes = sst->file.read_to_slice(sst->bloom_offset, bloom_size);

        auto bloom = BloomFilter::decode(bloom_bytes);
        sst->bloom_filter = std::make_shared<BloomFilter>(std::move(bloom));
    }

    // 3. 读取并解码元数据块
    uint32_t meta_size = sst->bloom_offset - sst->meta_block_offset;
    auto meta_bytes = sst->file.read_to_slice(sst->meta_block_offset, meta_size);
    sst->meta_entries = BlockMeta::decode_meta_from_slice(meta_bytes);

    // 4. 设置首尾key
    if (!sst->meta_entries.empty())
    {
        sst->first_key = sst->meta_entries.front().first_key;
        sst->last_key = sst->meta_entries.back().last_key;
    }

    return sst;
}

std::shared_ptr<Block> SST::read_block(size_t block_idx)
{
    if (block_idx >= meta_entries.size())
    {
        throw std::runtime_error("Block index out of range");
    }

    // 先从缓存池中查找
    if (cache != nullptr)
    {
        auto cached_block = cache->get(sst_id, block_idx);
        if (cached_block != nullptr)
        {
            return cached_block;
        }
    }
    else
    {
        throw std::runtime_error("Cache is nullptr");
    }

    const auto &meta = meta_entries[block_idx];
    size_t block_size;

    // 计算block的大小
    if (block_idx == meta_entries.size() - 1)
    {
        block_size = meta_block_offset - meta.offset;
    }
    else
    {
        block_size = meta_entries[block_idx + 1].offset - meta.offset;
    }

    // 读取block数据
    auto block_data = file.read_to_slice(meta.offset, block_size);
    auto block_res = Block::decode(block_data, true);

    // 更新缓存
    if (cache != nullptr)
    {
        cache->put(sst_id, block_idx, block_res);
    }
    else
    {
        throw std::runtime_error("Cache is nullptr");
    }

    return block_res;
}

size_t SST::find_block_idx(const std::string &key)
{
    // 先通过bloom filter判断
    if (!bloom_filter && !bloom_filter->possibly_contains(key))
    {
        return -1;
    }

    // 二分查找
    int left = 0, right = meta_entries.size() - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        const auto &meta = meta_entries[mid];

        if (key < meta.first_key)
        {
            right = mid - 1;
        }
        else if (key > meta.last_key)
        {
            left = mid + 1;
        }
        else
        {
            return mid;
        }
    }
    if (left >= meta_entries.size())
    {
        return -1;
    }

    return left;
}

std::string SST::get_first_key()
{
    return first_key;
}

std::string SST::get_last_key()
{
    return last_key;
}

size_t SST::sst_size() const
{
    return file.size();
}

size_t SST::get_sst_id() const
{
    return sst_id;
}

std::pair<uint64_t, uint64_t> SST::get_tranc_id_range() const
{
    return std::make_pair(min_tranc_id_, max_tranc_id_);
}

void SST::del_sst()
{
    file.del_file();
}