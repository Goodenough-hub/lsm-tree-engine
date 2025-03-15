#pragma once
#include "../block/block.h"
#include "../block/blockmeta.h"
#include "../block/block_cache.h"
#include "../utils/file.h"
#include "../utils/bloom_filter.h"
#include "sst_iterator.h"
#include <memory>
#include <vector>
#include <string>

class SSTBuilder;
class SstIterator;
class SST : public std::enable_shared_from_this<SST>
{
    friend class SSTBuilder;

private:
    FileObj file;
    std::vector<BlockMeta> meta_entries;
    size_t sst_id;
    uint32_t meta_block_offset; // 表示元数据块（Meta Block）在 SST 文件中的偏移量。
    // std::shared_ptr<BlockCache> cache;
    uint32_t bloom_offset;
    std::string first_key;
    std::string last_key;
    std::shared_ptr<BloomFilter> bloom_filter;
    std::shared_ptr<BlockCache> cache;

public:
    static std::shared_ptr<SST> open(size_t sst_id, FileObj file, std::shared_ptr<BlockCache> cache);
    std::shared_ptr<Block> read_block(size_t block_id);

    SstIterator get(const std::string &key);

    size_t num_blocks();

    SstIterator begin();
    SstIterator end();

    size_t find_block_idx(const std::string &key); // 返回-1表示没找到
};

class SSTBuilder
{
private:
    Block block; // 当前正在构建的block
    std::string first_key;
    std::string last_key;
    std::vector<BlockMeta> meta_entries;
    std::vector<uint8_t> data; // 已经编码的数据
    size_t block_size;         // block的容量，超出这个限制就被编码

public:
    std::shared_ptr<BloomFilter> bloom_filter;
    SSTBuilder(size_t block_size);
    void add(const std::string &key, const std::string &value);
    size_t estimated_size() const;
    void finish_block(); // 当前block被写满，然后清空进行下一个block的编码
    std::shared_ptr<SST> build(size_t sst_id, const std::string &path, std::shared_ptr<BlockCache> block_cache);
};