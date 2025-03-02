#include "../../include/block/blockmeta.h"
#include <cstring>

BlockMeta::BlockMeta() : offset(0), first_key(""), last_key("") {}

BlockMeta::BlockMeta(size_t offset, const std::string &first_key, const std::string &last_key)
    : offset(offset), first_key(first_key), last_key(last_key) {}

void BlockMeta::encode_meta_to_slice(std::vector<BlockMeta> &meta_entries, std::vector<uint8_t> &metadata) 
{
    // 将一组 BlockMeta 对象序列化为二进制格式的字节流metadata
    
    // 计算总大小 num_entries(32) + 所有 entries 的大小
    uint32_t num_entries = meta_entries.size();
    size_t total_size = sizeof(uint32_t);
    
    for(const auto &meta : meta_entries)
    {
        total_size += sizeof(uint32_t) // offset
                    + sizeof(uint16_t) // first_key_len
                    + meta.first_key.size() // first_key
                    + sizeof(uint16_t) // last_key_len
                    + meta.last_key.size(); // last_key
    }

    // 分配空间
    metadata.resize(total_size);
    uint8_t *ptr = metadata.data();

    // 写入元素个数
    memcpy(ptr, &num_entries, sizeof(uint32_t));
    ptr += sizeof(uint32_t);

    // 写入每个entry
    for(const auto &meta : meta_entries)
    {
        // offset
        uint32_t offset = meta.offset;
        memcpy(ptr, &offset, sizeof(uint32_t));
        ptr += sizeof(uint32_t);

        // first_key_len和first_key
        uint16_t first_key_len = meta.first_key.size();
        memcpy(ptr, &first_key_len, sizeof(uint16_t));
        ptr += sizeof(uint16_t);
        memcpy(ptr, meta.first_key.data(), first_key_len);

        // last_key_len和last_key
        uint16_t last_key_len = meta.last_key.size();
        memcpy(ptr, &last_key_len, sizeof(uint16_t));
        ptr += sizeof(uint16_t);
        memcpy(ptr, meta.last_key.data(), last_key_len);
    }
}