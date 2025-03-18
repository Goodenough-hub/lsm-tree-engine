#include "../../include/block/blockmeta.h"
#include <cstring>
#include <stdexcept>

BlockMeta::BlockMeta() : offset(0), first_key(""), last_key("") {}

BlockMeta::BlockMeta(size_t offset, const std::string &first_key, const std::string &last_key)
    : offset(offset), first_key(first_key), last_key(last_key) {}

void BlockMeta::encode_meta_to_slice(std::vector<BlockMeta> &meta_entries, std::vector<uint8_t> &metadata)
{
    // 将一组 BlockMeta 对象序列化为二进制格式的字节流metadata

    // 计算总大小 num_entries(32) + 所有 entries 的大小
    uint32_t num_entries = meta_entries.size();
    size_t total_size = sizeof(uint32_t);

    for (const auto &meta : meta_entries)
    {
        total_size += sizeof(uint32_t)        // offset
                      + sizeof(uint16_t)      // first_key_len
                      + meta.first_key.size() // first_key
                      + sizeof(uint16_t)      // last_key_len
                      + meta.last_key.size(); // last_key
    }
    total_size += sizeof(uint32_t); // hash

    // 分配空间
    metadata.resize(total_size);
    uint8_t *ptr = metadata.data();

    // 写入元素个数
    memcpy(ptr, &num_entries, sizeof(uint32_t));
    ptr += sizeof(uint32_t);

    // 写入每个entry
    for (const auto &meta : meta_entries)
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
        ptr += first_key_len;

        // last_key_len和last_key
        uint16_t last_key_len = meta.last_key.size();
        memcpy(ptr, &last_key_len, sizeof(uint16_t));
        ptr += sizeof(uint16_t);
        memcpy(ptr, meta.last_key.data(), last_key_len);
        ptr += last_key_len;
    }

    // 计算并写入hash
    const uint8_t *data_start = metadata.data() + sizeof(uint32_t);

    const uint8_t *data_end = ptr;
    size_t data_len = data_end - data_start;

    uint32_t computed_hash = std::hash<std::string_view>{}(
        std::string_view(reinterpret_cast<const char *>(data_start), data_len));
    memcpy(ptr, &computed_hash, sizeof(uint32_t));
}

std::vector<BlockMeta> BlockMeta::decode_meta_from_slice(const std::vector<uint8_t> &metadata)
{
    std::vector<BlockMeta> meta_entries;

    // 1.验证长度
    if (metadata.size() < sizeof(uint32_t) * 2)
    {
        throw std::runtime_error("metadata length error");
    }

    // 2.读取元素个数
    uint32_t num_entries;
    const uint8_t *ptr = metadata.data();
    memcpy(&num_entries, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);

    // 3.读取entries
    for (uint32_t i = 0; i < num_entries; i++)
    {
        BlockMeta meta;

        // 读取offset
        uint32_t offset32;
        memcpy(&offset32, ptr, sizeof(uint32_t));
        meta.offset = static_cast<size_t>(offset32);
        ptr += sizeof(uint32_t);

        // 读取first_key
        uint16_t first_key_len;
        memcpy(&first_key_len, ptr, sizeof(uint16_t));
        ptr += sizeof(uint16_t);
        meta.first_key.assign(reinterpret_cast<const char *>(ptr), first_key_len);
        ptr += first_key_len;

        // 读取last_key
        uint16_t last_key_len;
        memcpy(&last_key_len, ptr, sizeof(uint16_t));
        ptr += sizeof(uint16_t);

        // 将字节流中的 last_key 数据读取并赋值给 meta.last_key。
        meta.last_key.assign(reinterpret_cast<const char *>(ptr), last_key_len);

        ptr += last_key_len;

        meta_entries.push_back(meta);
    }

    // 4.验证hash
    uint32_t stored_hash;
    memcpy(&stored_hash, ptr, sizeof(uint32_t));

    const uint8_t *data_start = metadata.data() + sizeof(uint32_t);

    const uint8_t *data_end = ptr;
    size_t data_len = data_end - data_start;

    // 手动计算hash
    uint32_t computed_hash = std::hash<std::string_view>{}(std::string_view(reinterpret_cast<const char *>(data_start), data_len));

    if (stored_hash != computed_hash)
    {
        throw std::runtime_error("Invalid metadata hash");
    }

    return meta_entries;
}