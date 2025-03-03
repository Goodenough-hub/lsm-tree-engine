#pragma once
#include "../../include/block/block.h"
#include <stdexcept>
#include <vector>
std::vector<uint8_t> Block::encode() {
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

std::shared_ptr<Block> Block::decode(const std::vector<uint8_t> &encoded)
{
    // 创建对象
    auto block = std::make_shared<Block>();

    // 安全性检查
    if(encoded.size() < sizeof(uint16_t))
    {
        throw std::runtime_error("Invalid encoded block: size too small");
    }

    // 从后向前解析
    // 读取元素个数
    uint16_t num_elemts;
    size_t num_elemts_pos = encoded.size() - sizeof(uint16_t);
    memcpy(&num_elemts, encoded.data() +num_elemts_pos, sizeof(uint16_t));

    // 验证数据大小
    // 验证解码过程中接受到的编码数据encoded是否有足够的字节数来正确表示一个Block对象
    size_t required_size = sizeof(uint16_t) + num_elemts * sizeof(uint16_t);
    if(encoded.size() < required_size)
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

bool Block::is_empty() const
{
    return offsets.empty();
}

bool Block::add_entry(const std::string key, const std::string &value)
{
    if(cur_size() + key.size() + value.size()  + 3 * sizeof(uint16_t) > capacity)
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
