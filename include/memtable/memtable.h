#pragma once

#include "../skiplist/skiplist.h"
#include <list>
#include <shared_mutex>

class Memtable{
private:
    std::shared_ptr<SkipList> current_table;
    std::list<std::shared_ptr<SkipList>> frozen_tables;
    size_t frozen_bytes;

    std::shared_mutex rx_mutex; // 读写锁，以skiplist为单位
public:
    Memtable();
    ~Memtable() = default;
    void put(const std::string &key, const std::string &value);
    void remove(const std::string &key);
    void clear();
    std::optional<std::string> get(const std::string &key);
};