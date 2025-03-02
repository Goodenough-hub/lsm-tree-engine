#pragma once
#include <string>
#include <vector>

class BlockMeta
{
public:
    size_t offset; // block在sst文件中的偏移量
    std::string first_key;
    std::string last_key;

    BlockMeta();
    BlockMeta(size_t offset, const std::string& first_key, const std::string &last_key);

    static void encode_meta_to_slice(std::vector<BlockMeta> &meta_entries, std::vector<uint8_t> &metadata);

    static std::vector<BlockMeta> decode_meta_to_slice(const std::vector<uint8_t> &metadata);
};