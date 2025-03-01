#pragma once

#include "../skiplist/skiplist.h"
#include <list>
#include <shared_mutex>
#include <vector>

class Memtable{
private:
    std::shared_ptr<SkipList> current_table;
    std::list<std::shared_ptr<SkipList>> frozen_tables;
    size_t frozen_bytes;

    // std::shared_mutex rx_mutex; // 读写锁，以skiplist为单位
    // 两个锁
    std::shared_mutex cur_mtx; // 读写current_table的锁
    std::shared_mutex frozen_mtx; // 读写frozen_tables的锁
public:
    Memtable();
    ~Memtable() = default;
    void put(const std::string &key, const std::string &value);
    void put_batch(const std::vector<std::string> &keys, const std::vector<std::string> &values);
    void remove(const std::string &key);
    void remove_batch(const std::vector<std::string> &keys);
    void clear();
    std::optional<std::string> get(const std::string &key);
    std::vector<std::optional<std::string>> get_batch(const std::vector<std::string> &keys);
private:
    // 不加锁的版本
    void put_(const std::string &key, const std::string &value);
    void remove_(const std::string &key);
    std::optional<std::string> cur_get_(const std::string &key);
    std::optional<std::string> frozen_get_(const std::string &key);
};