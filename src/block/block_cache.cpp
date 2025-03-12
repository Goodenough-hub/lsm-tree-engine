#include "../../include/block/block_cache.h"
#include <mutex>
#include <unordered_map>

BlockCache::BlockCache(size_t capacity, size_t k) : capacity_(capacity), k_(k) {};

BlockCache::~BlockCache() = default;

void BlockCache::put(int sst_id, int block_id, std::shared_ptr<Block> block)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto key = std::make_pair(sst_id, block_id);
    auto it = cache_map_.find(key); // 返回的是 std::unordered_map 的迭代器

    if (it != cache_map_.end())
    {
        // 更新缓存
        // ！照理说block是只读的，因此实际的业务流程中不会触发这一个判断分支
        it->second->cache_block = block;
        update_access_count(it->second);
    }
    else
    {
        // 插入新的缓存项
        if (cache_map_.size() >= capacity_)
        {
            // 移除最久没有使用的缓存项
            if (!cache_list_less_k.empty())
            {
                // 优先移除访问次数不到k的缓存项
                cache_map_.erase(std::make_pair(cache_list_less_k.back().sst_id,
                                                cache_list_less_k.back().block_id));
                cache_list_less_k.pop_back();
            }
            else
            {
                cache_map_.erase(std::make_pair(cache_list_greater_k.back().sst_id,
                                                cache_list_greater_k.back().block_id));
                cache_list_greater_k.pop_back();
            }
        }
        CacheItem item{sst_id, block_id, block, 1};
        cache_list_less_k.push_front(item);
        cache_map_[key] = cache_list_less_k.begin();
    }
}

std::shared_ptr<Block> BlockCache::get(int sst_id, int block_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    ++total_request;
    auto key = std::make_pair(sst_id, block_id);
    auto it = cache_map_.find(key);

    if (it == cache_map_.end())
    {
        return nullptr;
    }
    ++hit_requests;
    update_access_count(it->second);
    return it->second->cache_block;
}

// 新缓存项的访问计数，并根据访问次数调整其在链表中的位置
void BlockCache::update_access_count(std::list<CacheItem>::iterator it)
{
    ++it->access_count;
    if (it->access_count < k_)
    {
        // 更新后仍然位于cache_list_less_k
        // 重置到链表头部，将 it 指向的元素移动到 cache_list_less_k 的开头位置。
        cache_list_less_k.splice(cache_list_less_k.begin(), cache_list_less_k, it);
    }
    else if (it->access_count == k_)
    {
        auto item = *it;
        cache_list_less_k.erase(it);
        cache_list_greater_k.push_front(item);
        cache_map_[std::make_pair(item.sst_id, item.block_id)] = cache_list_greater_k.begin();
    }
    else if (it->access_count > k_)
    {
        cache_list_greater_k.splice(cache_list_greater_k.begin(), cache_list_greater_k, it);
    }
}

double BlockCache::hit_rate()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return total_request == 0 ? 0.0 : (double)hit_requests / total_request;
}