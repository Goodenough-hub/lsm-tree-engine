#include "../../include/sst/sst.h"

SSTBuilder::SSTBuilder(size_t block_size) : block_size(block_size)
{
    meta_entries.clear();
    data.clear();
    first_key.clear();
    last_key.clear();
}

void SSTBuilder::add(const std::string &key, const std::string &value)
{
    if (first_key.empty())
    {
        first_key = key;
    }

    if (block.add_entry(key, value))
    {
        last_key = key;
        return;
    }

    finish_block(); // 把当前的block写入data中

    // 全新的block
    block.add_entry(key, value);
    first_key = key;
    last_key = key;
}

void SSTBuilder::finish_block()
{
    // 将block编码到data中
    auto old_block = std::move(this->block); // 移动语义
    auto encoded_block = old_block.encode();

    // 把block的原数据写入
    meta_entries.emplace_back(data.size(), first_key, last_key);

    // 计算哈希校验值
    auto block_hash = static_cast<uint32_t>(std::hash<std::string_view>{}(
        std::string_view(reinterpret_cast<char *>(
                             encoded_block.data()),
                         encoded_block.size())));

    // 预分配空间并添加数据
    data.reserve(data.size() + encoded_block.size() + sizeof(uint32_t)); // uint_32是哈希值
    data.insert(data.end(), encoded_block.begin(), encoded_block.end());
    data.resize(data.size() + sizeof(uint32_t));
    memcpy(data.data() + data.size() - sizeof(uint32_t), &block_hash, sizeof(uint32_t));
}

std::shared_ptr<SST> SSTBuilder::build(size_t sst_id, const std::string &path)
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

    // 编码成元数据块
    std::vector<uint8_t> meta_block;
    BlockMeta::encode_meta_to_slice(meta_entries, meta_block);

    // 计算元数据块的偏移量
    uint32_t meta_offset = static_cast<uint32_t>(data.size());

    // 构建文件内容
    // 1. 写入数据块
    std::vector<uint8_t> file_content = std::move(data);

    // 2. 写入元数据块
    file_content.insert(file_content.end(), meta_block.begin(), meta_block.end());

    // 3.布隆过滤器的优化

    // 构建SST对象并返回
    memcpy(file_content.data() + file_content.size() - sizeof(uint32_t), &meta_offset, sizeof(uint32_t));

    FileObj file;
    file.create_and_write(path, file_content);

    auto res = std::make_shared<SST>();

    res->sst_id = sst_id;
    res->file = std::move(file);
    res->first_key = meta_entries.front().first_key;
    res->last_key = meta_entries.back().last_key;
    res->meta_block_offset = meta_offset;

    // 缓存池

    return res;
}

SstIterator SST::get(const std::string &key)
{
    return SstIterator(shared_from_this(), key);
}

size_t SST::num_blocks()
{
    return meta_entries.size();
}

SstIterator SST::begin()
{
    return SstIterator(shared_from_this());
}

SstIterator SST::end()
{
    auto res = SstIterator(shared_from_this());
    res.m_sst = nullptr;
    res.m_block_idx = -1;
    res.cached_value = std::nullopt;
    return res;
}

std::shared_ptr<SST> SST::open(size_t sst_id, FileObj file)
{
    auto sst = std::make_shared<SST>();
    sst->sst_id = sst_id;
    sst->file = std::move(file);

    // 缓存池

    // 读取文件末尾的元数据块
    // 1.读取偏移量
    size_t file_size = sst->file.size();
    if (file_size < sizeof(uint32_t))
    {
        throw std::runtime_error("File size is too small");
    }

    auto meta_offset_bytes = sst->file.read_to_slice(file_size - sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&sst->meta_block_offset, meta_offset_bytes.data(), sizeof(uint32_t));

    // 2.读取并解码元数据
    uint32_t meta_size = file_size - sst->meta_block_offset;
    -sizeof(uint32_t);
    auto meta_bytes = sst->file.read_to_slice(sst->meta_block_offset, meta_size);
    sst->meta_entries = BlockMeta::decode_meta_to_slice(meta_bytes);

    // 3.设置首尾key
    if (!sst->meta_entries.empty())
    {
        sst->first_key = sst->meta_entries.front().first_key;
        sst->last_key = sst->meta_entries.back().last_key;
    }
    return sst;
}

std::shared_ptr<Block> SST::read_block(size_t block_id)
{
    if (block_id >= meta_entries.size())
    {
        throw std::runtime_error("Block id out of range");
    }

    const auto &meta = meta_entries[block_id];
    size_t block_size;

    // 计算block的大小
    if (block_id == meta_entries.size() - 1)
    {
        // 最后一个block
        // block_id 等于 meta_entries.size() - 1
        block_size = meta_block_offset - meta.offset;
    }
    else
    {
        block_size = meta_entries[block_id + 1].offset - meta.offset;
    }

    // 读取block
    auto block_data = file.read_to_slice(meta.offset, block_size);
    auto block_res = Block::decode(block_data, true);

    // 加入缓存

    return block_res;
}