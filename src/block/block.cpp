#pragma once
#include "../../include/block/block.h"
#include <stdexcept>
#include <vector>
#include <optional>

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

bool Block::add_entry(const std::string key, const std::string &value)
{
    if (cur_size() + key.size() + value.size() + 3 * sizeof(uint16_t) > capacity)
    {
        return false;
    }

    // 计算entries的大小 key_len + key + value_len + value
    size_t entry_size = key.size() + value.size() + 2 * sizeof(uint16_t);
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

std::optional<size_t> Block::get_idx_binary(const std::string &key)
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

std::optional<std::string> Block::get_value_binary(const std::string &key)
{
    auto idx = get_idx_binary(key);
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
std::optional<std::pair<std::shared_ptr<BlockIterator>, std::shared_ptr<BlockIterator>>> Block::get_monotony_predicate(std::function<int(const std::string &)> predicate)
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

BlockIterator Block::begin()
{
    return BlockIterator(shared_from_this(), 0);
}

BlockIterator Block::end()
{
    return BlockIterator(shared_from_this(), offsets.size());
}