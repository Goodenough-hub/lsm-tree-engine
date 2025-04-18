#pragma once
#include <memory>
#include <cstring>
#include <vector>
#include <functional>
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
        uint64_t tranc_id;
    };

    // 在指定偏移处解码键值对
    size_t get_offset_at(size_t idx) const;
    Entry get_entry_at(size_t offset) const;
    std::string get_key_at(size_t offset) const;
    std::string get_value_at(size_t offset) const;
    uint64_t get_tranc_id_at(size_t offset) const;
    int adjust_idx_by_tranc_id(size_t idx, uint64_t tranc_id);
    bool is_same_key(size_t idx, const std::string &target_key) const;

public:
    Block() = default;
    Block(size_t capacity);

    size_t cur_size() const; // 获取的是容量大小，而不是键值对的数量
    size_t size();           // 获取的是键值对的数量
    bool is_empty() const;

    std::vector<uint8_t> encode();
    static std::shared_ptr<Block> decode(const std::vector<uint8_t> &encode, bool with_hash = false);

    std::string get_first_key();
    std::optional<std::string> get_value_binary(const std::string &key, uint64_t tranc_id);

    bool add_entry(const std::string key, const std::string &value, uint64_t tranc_id, bool force_write);

    std::optional<size_t> get_idx_binary(const std::string &key, uint64_t tranc_id);
    int compare_key(size_t offset, const std::string &target);

    std::optional<std::pair<std::shared_ptr<BlockIterator>, std::shared_ptr<BlockIterator>>> get_monotony_predicate(uint64_t tranc_id, std::function<int(const std::string &)> predicate);

    BlockIterator begin(uint64_t tranc_id);
    BlockIterator end(uint64_t tranc_id);
};