#pragma once

#include "../memtable/memtable.h"
#include "../block/block_cache.h"
#include "../sst/sst.h"
#include "two_merge_iterator.h"
#include "transaction.h"
#include <memory>
#include <shared_mutex>
#include <cstring>
#include <unordered_map>
#include <optional>

class TranContext;

class LSMEngine
{
    friend class TranContext;

private:
    std::string data_dir;
    Memtable memtable;

    std::list<size_t> l0_sst_ids;                          // 有序列表，存储L0层的SSTable的ID
    std::unordered_map<size_t, std::shared_ptr<SST>> ssts; // 哈希表，通过SSTbale的ID来获取SSTable。提供高效的随机访问能力。

    std::shared_ptr<BlockCache> block_cache;

    std::shared_mutex ssts_mtx;

    void flush();
    void flush_all();
    std::string get_sst_path(size_t sst_id);

public:
    LSMEngine(std::string path);
    ~LSMEngine();
    void put(const std::string &key, const std::string &value, uint64_t tranc_id);
    void put_batch(const std::vector<std::pair<std::string, std::string>> &kvs, uint64_t tranc_id);
    std::optional<std::pair<std::string, uint64_t>> get(const std::string &key, uint64_t tranc_id);
    std::optional<std::pair<std::string, uint64_t>> sst_get_(const std::string &key, uint64_t tranc_id);
    void remove(const std::string &key, uint64_t tranc_id);
    void remove_batch(const std::vector<std::string> &keys, uint64_t tranc_id);

    void clear();

    std::optional<std::pair<TwoMergeIterator, TwoMergeIterator>> iter_monotony_predicate(uint64_t tranc_id, std::function<int(const std::string &)> predicate);
};

class LSM
{
public:
    // TODO: 实现WAL后修改启动流程
    LSM(std::string path);

    std::shared_ptr<TranContext> begin_transaction(const enum IsolationLevel &isolation_level);

private:
    std::shared_ptr<LSMEngine> engine_;
    std::shared_ptr<TranManager> tran_;
};