#pragma once
#include <memory>
#include <cstring>
#include <vector>

class Block 
{
    std::vector<uint8_t> data;
    std::vector<uint16_t> offsets;
    size_t capacity; // 容量

    struct Entry
    {
        std::string key;
        std::string value;
    };

    // 在指定偏移处解码键值对
    Entry get_entry_at(size_t offset) const;
    std::string get_key_at(size_t offset) const;
    std::string get_value_at(size_t offset) const;
    
public:
    Block() = default;
    Block(size_t capacity);

    size_t cur_size() const;
    bool is_empty() const;

    std::vector<uint8_t> encode();
    static std::shared_ptr<Block> decode(const std::vector<uint8_t> &encode);

    bool add_entry(const std::string key, const std::string &value);
};