#pragma once

// 定义一个缓存项

#include "block_iterator.h"
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

struct CacheItem
{
    int sst_id;
    int block_id;
    std::shared_ptr<Block> cache_block;
    uint64_t access_count;
};

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

// 自定义比较函数
struct pair_equal
{
    template <class T1, class T2>
    bool operator()(const std::pair<T1, T2> &p1, const std::pair<T1, T2> &p2) const
    {
        return p1.first == p2.first && p1.second == p2.second;
    }
};

class BlockCache
{
private:
    // 双向链表存储缓存项
    std::list<CacheItem> cache_list_greater_k;
    std::list<CacheItem> cache_list_less_k;

    // 哈希表存储缓存项
    // 键：std::pair<int, int>，表示由 sst_id 和 block_id 组成的键值对。
    // 值：std::list<CacheItem>::iterator，表示指向双向链表中缓存项的迭代器。
    // 哈希函数：pair_hash，用于计算键值对的哈希值。
    // 较函数：pair_equal，用于比较两个键值对是否相等。
    std::unordered_map<std::pair<int, int>, std::list<CacheItem>::iterator,
                       pair_hash, pair_equal>
        cache_map_;
    size_t capacity_;
    size_t k_;
    mutable std::mutex mutex_;
    mutable size_t total_request = 0;
    mutable size_t hit_requests = 0;

public:
    BlockCache(size_t capacity, size_t k);
    ~BlockCache();

    std::shared_ptr<Block> get(int sst_id, int block_id);
    void put(int sst_id, int block_id, std::shared_ptr<Block> block);

    void update_access_count(std::list<CacheItem>::iterator it);
    double hit_rate(); // 获取缓存命中率
};
