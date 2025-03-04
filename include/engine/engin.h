#pragma once

#include "../memtable/memtable.h"
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

    std::unordered_map<size_t, std::shared_ptr<SST>> ssts;

    std::shared_mutex mtx;

public:
    void open();
    void put();
    void get();
    void remove();
};