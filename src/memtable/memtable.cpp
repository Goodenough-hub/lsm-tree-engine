#include "../../include/memtable/memtable.h"
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>

Memtable::Memtable() : current_table(std::make_shared<SkipList>()), frozen_bytes(0) {}

void Memtable::put(const std::string &key, const std::string &value, uint64_t tranc_id)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    // current_table->put(key, value);
    put_(key, value, tranc_id);
}

void Memtable::put_(const std::string &key, const std::string &value, uint64_t tranc_id)
{
    current_table->put(key, value, tranc_id);
}

void Memtable::put_batch(const std::vector<std::pair<std::string, std::string>> &kvs, uint64_t tranc_id)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    for (size_t i = 0; i < kvs.size(); i++)
    {
        put_(kvs[i].first, kvs[i].second, tranc_id);
    }
}

void Memtable::remove(const std::string &key, uint64_t tranc_id)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    // current_table->put(key, "");
    remove_(key, tranc_id);
}

void Memtable::remove_(const std::string &key, uint64_t tranc_id)
{
    current_table->put(key, "", tranc_id);
}

void Memtable::remove_batch(const std::vector<std::string> &keys, uint64_t tranc_id)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    for (const auto &key : keys)
    {
        remove_(key, tranc_id);
    }
}

void Memtable::clear()
{
    std::unique_lock<std::shared_mutex> lock1(frozen_mtx);
    std::unique_lock<std::shared_mutex> lock2(cur_mtx);
    frozen_tables.clear();
    current_table->clear();
    frozen_bytes = 0;
}

// 谓词查询
std::optional<std::pair<HeapIterator, HeapIterator>> Memtable::iter_monotony_predicate(uint64_t tranc_id, std::function<int(const std::string &)> predicate)
{
    // 汇总每个skiplist谓词查询的结果

    // 加锁，并发读取
    std::shared_lock<std::shared_mutex> lock1(frozen_mtx);
    std::shared_lock<std::shared_mutex> lock2(cur_mtx);

    std::vector<SearchItem> item_vec; // 存储满足谓词条件的查询结果。每个结果是一个SearchItem对象，包含键、值以及表的索引。

    // 对活跃表的执行谓词查询
    auto cur_result = current_table->iters_monotony_predicate(predicate);
    if (cur_result.has_value())
    {
        auto [cur_begin, cur_end] = cur_result.value();
        for (auto iter = cur_begin; iter != cur_end; ++iter) // 遍历迭代器范围
        {
            if (tranc_id != 0 && iter.get_tranc_id() == tranc_id > tranc_id)
            {
                continue;
            }
            item_vec.emplace_back(iter.get_key(), iter.get_value(), 0, 0, iter.get_tranc_id());
        }
    }

    int table_idx = 1;
    for (auto ft = frozen_tables.begin(); ft != frozen_tables.end(); ft++) // 遍历frozen_tables中的每个冻结表
    {
        auto table = *ft;
        auto result = table->iters_monotony_predicate(predicate);
        if (result.has_value())
        {
            auto [begin, end] = result.value();
            for (auto iter = begin; iter != end; ++iter)
            {
                if (tranc_id != 0 && iter.get_tranc_id() == tranc_id > tranc_id)
                {
                    continue;
                }
                item_vec.emplace_back(iter.get_key(), iter.get_value(), table_idx, 0, iter.get_tranc_id());
            }
        }
        table_idx++;
    }

    return std::make_pair(HeapIterator(item_vec, tranc_id), HeapIterator());
}

SkipListIterator Memtable::get(const std::string &key, uint64_t tranc_id)
{
    // current table中找
    std::shared_lock<std::shared_mutex> lock2(frozen_mtx);
    std::shared_lock<std::shared_mutex> lock1(cur_mtx);

    return get_(key, tranc_id);
}

SkipListIterator Memtable::get_(const std::string &key, uint64_t tranc_id)
{
    auto result1 = cur_get_(key, tranc_id);
    if (result1.is_valid())
    {
        return result1;
    }

    return frozen_get_(key, tranc_id);
}

std::vector<SkipListIterator> Memtable::get_batch(const std::vector<std::string> &keys, uint64_t tranc_id)
{
    // 优先级较高，持有两把锁
    // 先上frozen_mtx锁，再上cur_mtx锁。是为了避免死锁
    std::shared_lock<std::shared_mutex> lock1(frozen_mtx);
    std::shared_lock<std::shared_mutex> lock2(cur_mtx);

    std::vector<SkipListIterator> results;

    for (const auto &key : keys)
    {

        auto result1 = cur_get_(key, tranc_id);
        if (result1.is_valid())
        {
            results.push_back(result1);
            continue;
        }

        auto result2 = frozen_get_(key, tranc_id);
        results.push_back(result2);
    }

    return results;
}

SkipListIterator Memtable::cur_get_(const std::string &key, uint64_t tranc_id)
{
    auto res = current_table->get(key, tranc_id);
    if (res.is_valid())
    {
        return res;
    }
    return SkipListIterator{nullptr};
}

SkipListIterator Memtable::frozen_get_(const std::string &key, uint64_t tranc_id)
{
    // 查冻结的表
    for (auto &table : frozen_tables)
    {
        auto res = table->get(key, tranc_id);
        if (res.is_valid())
            return res;
    }
    return SkipListIterator{nullptr};
}

size_t Memtable::get_cur_size()
{
    std::shared_lock<std::shared_mutex> lock(cur_mtx);
    return current_table->get_size();
}
size_t Memtable::get_frozen_size()
{
    std::shared_lock<std::shared_mutex> lock(frozen_mtx);
    return frozen_bytes;
}
size_t Memtable::get_total_size()
{
    return get_cur_size() + get_frozen_size();
}

void Memtable::frozen_cur_table()
{
    std::unique_lock<std::shared_mutex> lock1(frozen_mtx);
    std::unique_lock<std::shared_mutex> lock2(cur_mtx);
    frozen_cur_table_();
}

void Memtable::frozen_cur_table_()
{
    frozen_bytes += current_table->get_size();
    frozen_tables.push_front(std::move(current_table)); // 最近插入的表插入队头
    current_table = std::make_shared<SkipList>();
}
std::shared_ptr<SST> Memtable::flush_last(SSTBuilder &builder, std::string &sst_path, size_t sst_id, std::shared_ptr<BlockCache> block_cache)
{
    std::unique_lock<std::shared_mutex> lock1(frozen_mtx);
    if (frozen_tables.empty())
    {
        // 如果为空，将活跃表刷入
        // 活跃表为空，直接返回

        std::unique_lock<std::shared_mutex> lock2(cur_mtx);
        if (current_table->get_size() == 0)
        {
            return nullptr;
        }

        frozen_tables.push_front(current_table); // 最近插入的表插入队头
        frozen_bytes += current_table->get_size();
        current_table = std::make_shared<SkipList>();
    }

    std::shared_ptr<SkipList> table = frozen_tables.back(); // 取出插入时间最长的表
    frozen_tables.pop_back();
    frozen_bytes -= table->get_size();

    auto flush_data = table->flush(); // 获取所有键值对
    for (auto &[k, v, t] : flush_data)
    {
        builder.add(k, v, t);
    }

    auto sst = builder.build(sst_id, sst_path, block_cache);
    return sst;
}

HeapIterator Memtable::begin(uint64_t tranc_id)
{
    std::shared_lock<std::shared_mutex> lock1(frozen_mtx);
    std::shared_lock<std::shared_mutex> lock2(cur_mtx);

    std::vector<SearchItem> item_vec;
    for (auto iter = current_table->begin(); iter != current_table->end(); ++iter)
    {
        if (tranc_id != 0 && iter.get_tranc_id() != tranc_id)
        {
            continue;
        }
        item_vec.emplace_back(iter.get_key(), iter.get_value(), 0, iter.get_tranc_id());
    }

    int table_idx = 1;
    for (auto ft = frozen_tables.begin(); ft != frozen_tables.end(); ft++)
    {
        auto table = *ft;
        for (auto iter = table->begin(); iter != table->end(); ++iter)
        {
            if (tranc_id != 0 && iter.get_tranc_id() != tranc_id)
            {
                continue;
            }
            item_vec.emplace_back(iter.get_key(), iter.get_value(), table_idx, 0, iter.get_tranc_id());
        }
        table_idx++;
    }
    return HeapIterator(item_vec, tranc_id);
}

HeapIterator Memtable::end(uint64_t tranc_id)
{
    return HeapIterator();
}