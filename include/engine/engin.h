#pragma once

#include "../memtable/memtable.h"
#include "../block/block_cache.h"
#include "../sst/sst.h"
#include <memory>
#include <shared_mutex>
#include <cstring>
#include <unordered_map>

class LSMEngine
{
private:
    std::string data_dir;
    Memtable memtable;

    std::list<size_t> l0_sst_ids;                          // 有序列表，存储L0层的SSTable的ID
    std::unordered_map<size_t, std::shared_ptr<SST>> ssts; // 哈希表，通过SSTbale的ID来获取SSTable。提供高效的随机访问能力。

    std::shared_ptr<BlockCache> block_cache;

    std::shared_mutex ssts_mtx;

    void flush();
    std::string get_sst_path(size_t sst_id);

public:
    LSMEngine(std::string path);
    void put(const std::string &key, const std::string &value);
    std::optional<std::string> get(const std::string &key);
    void remove();
};