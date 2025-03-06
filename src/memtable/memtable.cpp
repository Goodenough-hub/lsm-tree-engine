#include "../../include/memtable/memtable.h"
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>

Memtable::Memtable() : current_table(std::make_shared<SkipList>()), frozen_bytes(0) {}

void Memtable::put(const std::string &key, const std::string &value)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    // current_table->put(key, value);
    put_(key, value);
}

void Memtable::put_(const std::string &key, const std::string &value)
{
    current_table->put(key, value);
}

void Memtable::put_batch(const std::vector<std::string> &key, const std::vector<std::string> &value)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    for (size_t i = 0; i < key.size(); i++)
    {
        put_(key[i], value[i]);
    }
}

void Memtable::remove(const std::string &key)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    // current_table->put(key, "");
    remove_(key);
}

void Memtable::remove_(const std::string &key)
{
    current_table->put(key, "");
}

void Memtable::remove_batch(const std::vector<std::string> &keys)
{
    std::unique_lock<std::shared_mutex> lock(cur_mtx);
    for (const auto &key : keys)
    {
        remove_(key);
    }
}

void Memtable::clear()
{
    // ! 为了避免死锁, 需要按照顺序加锁, 先获取 cur_mtx 再获取 frozen_mtx
    std::unique_lock<std::shared_mutex> lock1(cur_mtx);
    std::unique_lock<std::shared_mutex> lock2(frozen_mtx);
    frozen_tables.clear();
    current_table->clear();
    frozen_bytes = 0;
}

std::optional<std::string> Memtable::get(const std::string &key)
{
    // current table中找
    std::shared_lock<std::shared_mutex> lock1(cur_mtx);

    auto result1 = cur_get_(key);
    if (result1.has_value())
    {
        return result1;
    }

    lock1.unlock();

    // frozen table中找
    std::shared_lock<std::shared_mutex> lock2(frozen_mtx);

    return frozen_get_(key);
}

std::vector<std::optional<std::string>> Memtable::get_batch(const std::vector<std::string> &keys)
{
    // 优先级较高，持有两把锁
    std::shared_lock<std::shared_mutex> lock1(cur_mtx);
    std::shared_lock<std::shared_mutex> lock2(frozen_mtx);

    std::vector<std::optional<std::string>> results;

    for (const auto &key : keys)
    {

        auto result1 = cur_get_(key);
        if (result1.has_value())
        {
            results.push_back(result1);
            continue;
        }

        auto result2 = frozen_get_(key);
        results.push_back(result2);
    }

    return results;
}

std::optional<std::string> Memtable::cur_get_(const std::string &key)
{
    auto res = current_table->get(key);
    if (res.has_value())
    {
        auto data = res.value();
        return data;
    }
    return std::nullopt;
}

std::optional<std::string> Memtable::frozen_get_(const std::string &key)
{
    // 查冻结的表
    for (auto &table : frozen_tables)
    {
        auto res = table->get(key);
        if (res.has_value())
            return res.value();
    }
    return std::nullopt;
}

size_t Memtable::get_cur_size()
{
    return current_table->get_size();
}
size_t Memtable::get_frozen_size()
{
    return frozen_bytes;
}
size_t Memtable::get_total_size()
{
    return get_cur_size() + get_frozen_size();
}

std::shared_ptr<SST> Memtable::flush_last(SSTBuilder &builder, std::string &sst_path, size_t sst_id)
{
    if (frozen_tables.empty())
    {
        // 如果为空，将活跃表刷入
        // 活跃表为空，直接返回
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
    for (auto &[k, v] : flush_data)
    {
        builder.add(k, v);
    }

    auto sst = builder.build(sst_id, sst_path);
    return sst;
}