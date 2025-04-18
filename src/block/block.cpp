#pragma once
#include "../../include/block/block.h"
#include <stdexcept>
#include <vector>
#include <optional>

// 从Block的数据中提取指定偏移量位置的事务ID
uint64_t Block::get_tranc_id_at(size_t offset) const
{
    uint64_t key_len; // 存储key长度
    memcpy(&key_len, data.data() + offset, sizeof(uint16_t));

    // 计算value长度的位置
    size_t value_len_ops = offset + sizeof(uint16_t) + key_len;
    uint16_t value_len;
    memcpy(&value_len, data.data() + value_len_ops, sizeof(uint16_t));

    // 计算事务id的位置
    size_t tranc_id_ops = value_len_ops + sizeof(uint16_t) + value_len;
    uint64_t tranc_id;
    memcpy(&tranc_id, data.data() + tranc_id_ops, sizeof(uint64_t));
    return tranc_id;
}

Block::Block(size_t capacity) : capacity(capacity) {}

std::vector<uint8_t> Block::encode()
{
    // 计算总大小
    size_t total_bytes = data.size() + offsets.size() * sizeof(uint16_t) + sizeof(uint16_t);

    std::vector<uint8_t> encoded(total_bytes, 0);

    // 1.复制data
    memcpy(encoded.data(), data.data(), data.size() * sizeof(uint8_t));

    // 2.复制offsets
    size_t offset_pos = data.size();
    memcpy(encoded.data() + offset_pos, offsets.data(), offsets.size() * sizeof(uint16_t));

    // 3.写入extra
    size_t num_pos = data.size() + offsets.size() * sizeof(uint16_t);
    uint16_t num_elements = offsets.size();
    memcpy(encoded.data() + num_pos, &num_elements, sizeof(uint16_t));

    return encoded;
}

std::shared_ptr<Block> Block::decode(const std::vector<uint8_t> &encoded, bool with_hash)
{
    // 创建对象
    auto block = std::make_shared<Block>();

    // 安全性检查
    if (encoded.size() < sizeof(uint16_t))
    {
        throw std::runtime_error("Invalid encoded block: size too small");
    }

    // 从后向前解析
    // 读取元素个数
    uint16_t num_elemts;
    size_t num_elemts_pos = encoded.size() - sizeof(uint16_t);

    if (with_hash)
    {
        num_elemts_pos -= sizeof(uint32_t);
        auto hash_pos = encoded.size() - sizeof(uint32_t);
        uint32_t hash_value;
        memcpy(&hash_value, encoded.data() + hash_pos, sizeof(uint32_t));

        uint32_t compute_hash = std::hash<std::string_view>{}(
            std::string_view(reinterpret_cast<const char *>(encoded.data()),
                             encoded.size() - sizeof(uint32_t)));

        if (hash_value != compute_hash)
        {
            throw std::runtime_error("Invalid encoded block: hash value mismatch");
        }
    }

    memcpy(&num_elemts, encoded.data() + num_elemts_pos, sizeof(uint16_t));

    // 验证数据大小
    // 验证解码过程中接受到的编码数据encoded是否有足够的字节数来正确表示一个Block对象
    size_t required_size = sizeof(uint16_t) + num_elemts * sizeof(uint16_t);
    if (encoded.size() < required_size)
    {
        throw std::runtime_error("Invalid encoded data size");
    }

    // 计算各部分的位置
    size_t offset_section_start = num_elemts_pos - num_elemts * sizeof(uint16_t);

    // 读取偏移数组
    block->offsets.resize(num_elemts);
    memcpy(block->offsets.data(), encoded.data() + offset_section_start, num_elemts * sizeof(uint16_t));

    // 复制数据段
    block->data.resize(offset_section_start);
    block->data.assign(encoded.begin(), encoded.begin() + offset_section_start);

    return block;
}

size_t Block::cur_size() const
{
    return data.size() + offsets.size() * sizeof(uint16_t) + sizeof(uint16_t);
}

size_t Block::size()
{
    return offsets.size();
}

bool Block::is_empty() const
{
    return offsets.empty();
}

// tranc_id：事务ID，需要同步持久化
// 事务id持久化时必然时正整数
// 事务id为0时，表示不开启事务功能，但不可能出现在实际的文件持久化内容中
bool Block::add_entry(const std::string key, const std::string &value, uint64_t tranc_id, bool force_write)
{
    if (!force_write && cur_size() + key.size() + value.size() + 3 * sizeof(uint16_t) > capacity)
    {
        return false;
    }

    // 计算entries的大小 key_len + key + value_len + value + uint64_t
    // (事务ID的内容)
    size_t entry_size = key.size() + value.size() + 2 * sizeof(uint16_t) + sizeof(uint64_t);
    size_t old_size = data.size();
    data.resize(old_size + entry_size);

    // 写入key_len, key
    uint16_t key_len = key.size();
    mempcpy(data.data() + old_size, &key_len, sizeof(uint16_t));
    memcpy(data.data() + old_size + sizeof(uint16_t), key.data(), key_len);

    // 写入value_len, value
    uint16_t value_len = value.size();
    memcpy(data.data() + old_size + sizeof(uint16_t) + key_len, &value_len, sizeof(uint16_t));
    memcpy(data.data() + old_size + sizeof(uint16_t) + key_len + sizeof(uint16_t), value.data(), value_len);

    // 写入事务id
    memcpy(data.data() + old_size + sizeof(uint16_t) + key_len + sizeof(uint16_t) + value_len, &tranc_id, sizeof(uint64_t));

    // 记录偏移量
    offsets.push_back(old_size);
    return true;
}

size_t Block::get_offset_at(size_t idx) const
{
    if (idx > offsets.size())
    {
        throw std::runtime_error("idx out of range");
    }
    return offsets[idx];
}

Block::Entry Block::get_entry_at(size_t offset) const
{
    Entry entry;
    entry.key = get_key_at(offset);
    entry.value = get_value_at(offset);
    return entry;
}

std::string Block::get_key_at(size_t offset) const
{
    uint16_t key_len;
    memcpy(&key_len, data.data() + offset, sizeof(uint16_t));
    return std::string(reinterpret_cast<const char *>(data.data() + offset + sizeof(uint16_t)), key_len);
}

std::string Block::get_value_at(size_t offset) const
{
    uint16_t key_len;
    memcpy(&key_len, data.data() + offset, sizeof(uint16_t));
    uint16_t value_len;
    memcpy(&value_len, data.data() + offset + sizeof(uint16_t) + key_len, sizeof(uint16_t));
    return std::string(reinterpret_cast<const char *>(data.data() + offset + sizeof(uint16_t) + key_len + sizeof(uint16_t)), value_len);
}

bool Block::is_same_key(size_t idx, const std::string &target_key) const
{
    if (idx >= offsets.size())
    {
        return false;
    }

    return get_key_at(offsets[idx]) == target_key;
}

// 相同的key是连续分布的, 且相同key按照事务id由大到小排布
// 这里的逻辑是找到最接近 tranc_id 的键值对的索引位置
// example:
// tranc: 100
// get (key1, 100)
// key1: value1 97
// key1: value2 98
// key1: value3 101
//
// tranc_id = 0表示不开启事务可见性的限制
int Block::adjust_idx_by_tranc_id(size_t idx, uint64_t tranc_id)
{
    if (idx >= offsets.size())
    {
        return -1; // 索引越界
    }

    auto target_key = get_key_at(offsets[idx]);

    if (tranc_id != 0)
    {
        auto cur_tranc_id = get_tranc_id_at(offsets[idx]);
        if (cur_tranc_id <= tranc_id)
        {
            // 当前事务可见的，需要向前继续判断
            // 向前判断的前提是key相同
            size_t pre_idx = idx;
            while (pre_idx > 0 && is_same_key(pre_idx - 1, target_key))
            {
                pre_idx--;
                auto new_tranc_id = get_tranc_id_at(offsets[pre_idx]);
                if (new_tranc_id > tranc_id)
                {
                    // 不可见，返回之前位置的索引
                    return pre_idx + 1;
                }
            }
            return pre_idx;
        }
        else
        {
            // 当前记录不可见，向后查找
            size_t next_idx = idx + 1;
            while (next_idx < offsets.size() && is_same_key(next_idx, target_key))
            {
                auto new_tranc_id = get_tranc_id_at(offsets[next_idx]);
                if (new_tranc_id <= tranc_id)
                {
                    return next_idx; // 找到可见记录
                }
                next_idx++;
            }
            return -1;
        }
    }
    else
    {
        // 没有开启事务的情况
        size_t pre_idx = idx;
        while (pre_idx > 0 && is_same_key(pre_idx - 1, target_key))
        {
            pre_idx--;
        }
        return pre_idx;
    }
}

std::optional<size_t> Block::get_idx_binary(const std::string &key, uint64_t tranc_id)
{
    if (offsets.empty())
    {
        return std::nullopt;
    }

    // 二分查找
    int left = 0;
    int right = offsets.size() - 1;

    while (left <= right)
    {
        int mid = (left + right) / 2;
        size_t mid_offset = offsets[mid];

        int cmp = compare_key(mid_offset, key);

        if (cmp == 0)
        {
            // 找到了key，但还需要判断事务id的可见性
            auto new_mid = adjust_idx_by_tranc_id(mid, tranc_id);
            if (new_mid != -1)
            {
                return std::nullopt; // 没有找到可见的记录
            }
            return mid;
        }
        else if (cmp < 0)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return std::nullopt;
}

int Block::compare_key(size_t offset, const std::string &target)
{
    std::string key = get_key_at(offset);

    return key.compare(target);
}

std::string Block::get_first_key()
{
    if (data.empty() || offsets.empty())
    {
        return "";
    }

    // 读取第一个key
    uint16_t key_len;
    memcpy(&key_len, data.data(), sizeof(uint16_t)); // 从data的起始位置读取一个uint16_t类型的值作为key的长度。

    return std::string(reinterpret_cast<const char *>(data.data() + sizeof(uint16_t)), key_len); // 根据读取的长度，从data中提取对应的字节序列并转换为字符串返回。
}

std::optional<std::string> Block::get_value_binary(const std::string &key, uint64_t tranc_id)
{
    auto idx = get_idx_binary(key, tranc_id);
    if (idx.has_value())
    {
        return get_value_at(offsets[idx.value()]);
    }
    return std::nullopt;
}

// 返回的是第一个满足谓词的位置，和最后一个满足谓词位置的下一个位置
// 左闭右开区间
// predicated 返回值：
// 0：满足条件
// >0：不满足谓词，需要向右移动
// <0：不满足谓词，需要向左移动
std::optional<std::pair<std::shared_ptr<BlockIterator>, std::shared_ptr<BlockIterator>>> Block::get_monotony_predicate(uint64_t tranc_id, std::function<int(const std::string &)> predicate)
{
    // 如果offsets为空，则表示当前块中没有数据，直接返回
    if (offsets.empty())
    {
        return std::nullopt;
    }

    // 第一次二分查找到第一个满足谓词的位置
    int left = 0;
    int right = offsets.size() - 1;
    int first = -1;       // 真正的区间的起始位置
    int first_first = -1; // 第一找到的谓词位置

    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        size_t mid_offset = offsets[mid];

        auto mid_key = get_key_at(mid_offset);
        int direction = predicate(mid_key);

        if (direction < 0)
        {
            // 目标在mid左侧
            right = mid - 1;
        }
        else if (direction > 0)
        {
            // 在目标mid右侧
            left = mid + 1;
        }
        else
        {
            // 目标在mid位置
            first = mid;
            if (first_first == -1)
            {
                first_first = mid;
            }
            // 继续判断左边是否符合
            right = mid - 1;
        }
    }

    if (first == -1)
    {
        return std::nullopt;
    }

    // 继续找到最后一个满足谓词的位置
    left = first_first;
    right = offsets.size() - 1;
    int last = -1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        size_t mid_offset = offsets[mid];

        auto mid_key = get_key_at(mid_offset);
        int direction = predicate(mid_key);

        if (direction < 0)
        {
            // 目标在mid左侧
            right = mid - 1;
        }
        else if (direction > 0)
        {
            // 目标在mid右侧
            throw std::runtime_error("block is not sorted");
        }
        else
        {
            // 目标在mid位置
            last = mid;
            // 继续判断右边是否符合
            left = mid + 1;
        }
    }
    auto it_begin = std::make_shared<BlockIterator>(shared_from_this(), last);
    auto it_end = std::make_shared<BlockIterator>(shared_from_this(), last + 1);

    return std::make_pair(it_begin, it_end);
}

BlockIterator Block::begin(uint64_t tranc_id)
{
    return BlockIterator(shared_from_this(), 0, tranc_id);
}

BlockIterator Block::end(uint64_t tranc_id)
{
    return BlockIterator(shared_from_this(), offsets.size(), tranc_id);
}