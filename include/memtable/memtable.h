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
    void put(const std::string &key, const std::string &value);
    void put_batch(const std::vector<std::string> &keys, const std::vector<std::string> &values);
    void remove(const std::string &key);
    void remove_batch(const std::vector<std::string> &keys);
    void clear();

    // 迭代器
    HeapIterator begin();
    HeapIterator end();

    // 补全谓词查询
    std::optional<std::pair<HeapIterator, HeapIterator>> iter_monotony_predicate(std::function<int(const std::string &)> predicate);

    std::optional<std::string> get(const std::string &key);
    std::vector<std::optional<std::string>> get_batch(const std::vector<std::string> &keys);

    size_t get_cur_size();
    size_t get_frozen_size();
    size_t get_total_size();

    // 构建SST
    std::shared_ptr<SST> flush_last(SSTBuilder &builder, std::string &sst_path, size_t sst_id);

private:
    // 不加锁的版本
    void put_(const std::string &key, const std::string &value);
    void remove_(const std::string &key);
    std::optional<std::string> cur_get_(const std::string &key);
    std::optional<std::string> frozen_get_(const std::string &key);
};