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
    friend std::optional<std::pair<SstIterator, SstIterator>> sst_iters_monotony_predicate(std::shared_ptr<SST> sst,
                                                                                           uint64_t max_tranc_id, std::function<int(const std::string &)> predicate);
    friend class SSTBuilder;

private:
    FileObj file;

    size_t sst_id;
    uint32_t meta_block_offset; // 表示元数据块（Meta Block）在 SST 文件中的偏移量。
    // std::shared_ptr<BlockCache> cache;
    uint32_t bloom_offset;
    std::string first_key;
    std::string last_key;
    std::shared_ptr<BloomFilter> bloom_filter;
    std::shared_ptr<BlockCache> cache;
    uint64_t min_tranc_id_ = UINT64_MAX;
    uint64_t max_tranc_id_ = 0;

public:
    std::vector<BlockMeta> meta_entries;

    static std::shared_ptr<SST> open(size_t sst_id, FileObj file, std::shared_ptr<BlockCache> cache);
    std::shared_ptr<Block> read_block(size_t block_id);

    SstIterator get(const std::string &key, uint64_t tranc_id);

    size_t num_blocks();

    SstIterator begin(uint64_t tranc_id);
    SstIterator end(uint64_t tranc_id);

    size_t find_block_idx(const std::string &key); // 返回-1表示没找到

    std::string get_first_key();
    std::string get_last_key();
    size_t sst_size() const;
    size_t get_sst_id() const;

    std::pair<uint64_t, uint64_t> get_tranc_id_range() const;

    void del_sst();
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
    uint64_t min_tranc_id_ = UINT64_MAX;
    uint64_t max_tranc_id_ = 0;

public:
    std::shared_ptr<BloomFilter> bloom_filter;
    SSTBuilder(size_t block_size, bool with_bloom);
    void add(const std::string &key, const std::string &value, uint64_t tranc_id);
    size_t estimated_size() const;
    void finish_block(); // 当前block被写满，然后清空进行下一个block的编码
    std::shared_ptr<SST> build(size_t sst_id, const std::string &path, std::shared_ptr<BlockCache> block_cache);
};