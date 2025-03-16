#pragma once
#include <memory>
#include <cstring>
#include <vector>
#include "block_iterator.h"

class Block : public std::enable_shared_from_this<Block>
{
    friend BlockIterator;
    std::vector<uint8_t> data;
    std::vector<uint16_t> offsets;
    size_t capacity; // 容量

    struct Entry
    {
        std::string key;
        std::string value;
    };

    // 在指定偏移处解码键值对
    size_t get_offset_at(size_t idx) const;
    Entry get_entry_at(size_t offset) const;
    std::string get_key_at(size_t offset) const;
    std::string get_value_at(size_t offset) const;

public:
    Block() = default;
    Block(size_t capacity);

    size_t cur_size() const; // 获取的是容量大小，而不是键值对的数量
    size_t size();           // 获取的是键值对的数量
    bool is_empty() const;

    std::vector<uint8_t> encode();
    static std::shared_ptr<Block> decode(const std::vector<uint8_t> &encode, bool with_hash = false);

    std::string get_first_key();
    std::optional<std::string> get_value_binary(const std::string &key);

    bool add_entry(const std::string key, const std::string &value);

    std::optional<size_t> get_idx_binary(const std::string &key);
    int compare_key(size_t offset, const std::string &target);

    BlockIterator begin();
    BlockIterator end();
};