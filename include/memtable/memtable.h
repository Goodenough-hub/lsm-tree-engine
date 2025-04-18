#pragma once

#include "../skiplist/skiplist.h"
#include "../../include/iterator/iterator.h"
#include "../sst/sst.h"
#include <list>
#include <shared_mutex>
#include <vector>

class Memtable
{
private:
    std::shared_ptr<SkipList> current_table;
    std::list<std::shared_ptr<SkipList>> frozen_tables;
    size_t frozen_bytes;

    // std::shared_mutex rx_mutex; // 读写锁，以skiplist为单位
    // 两个锁
    std::shared_mutex cur_mtx;    // 读写current_table的锁
    std::shared_mutex frozen_mtx; // 读写frozen_tables的锁
public:
    Memtable();
    ~Memtable() = default;
    void put(const std::string &key, const std::string &value, uint64_t tranc_id);
    void put_batch(const std::vector<std::pair<std::string, std::string>> &kvs, uint64_t tranc_id);
    void remove(const std::string &key, uint64_t tranc_id);
    void remove_batch(const std::vector<std::string> &keys, uint64_t tranc_id);
    void clear();

    // 迭代器
    HeapIterator begin(uint64_t tranc_id);
    HeapIterator end(uint64_t tranc_id);

    // 谓词查询
    std::optional<std::pair<HeapIterator, HeapIterator>> iter_monotony_predicate(uint64_t tranc_id, std::function<int(const std::string &)> predicate);

    SkipListIterator get(const std::string &key, uint64_t tranc_id);
    std::vector<SkipListIterator> get_batch(const std::vector<std::string> &keys, uint64_t tranc_id);

    size_t get_cur_size();
    size_t get_frozen_size();
    size_t get_total_size();

    // 构建SST
    std::shared_ptr<SST> flush_last(SSTBuilder &builder, std::string &sst_path, size_t sst_id, std::shared_ptr<BlockCache> block_cache);

    void frozen_cur_table();

private:
    // 不加锁的版本
    void put_(const std::string &key, const std::string &value, uint64_t tranc_id);
    void remove_(const std::string &key, uint64_t tranc_id);
    SkipListIterator cur_get_(const std::string &key, uint64_t tranc_id);
    SkipListIterator frozen_get_(const std::string &key, uint64_t tranc_id);
    void frozen_cur_table_();
};