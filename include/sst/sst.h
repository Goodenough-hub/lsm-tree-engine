#pragma once
#include "../block/block.h"
#include "../block/blockmeta.h"
#include <memory>
#include <vector>
#include <string>

class SST
{
private:
    // FileOBJ
    std::vector<BlockMeta> meta_entries;
    size_t sst_id;
    uint32_t meta_block_offset;
    std::string first_key;
    std::string last_key;

public:
    std::shared_ptr<Block> read_block(size_t block_id) const;
};

class SSTBuilder
{
private:
    Block block; // 当前正在构建的block
    std::string first_key;
    std::string last_key;
    std::vector<BlockMeta> meta_entries;
    std::vector<uint8_t> data; // 已经编码的数据
    size_t block_size; // block的容量，超出这个限制就被编码

public:
    SSTBuilder(size_t block_size);
    void add(const std::string &key, const std::string &value);
    size_t estimated_size() const;
    void finish_block(); // 当前block被写满，然后清空进行下一个block的编码
    std::shared_ptr<SST> build(size_t sst_id, const std::string &path);
};