#include "../../include/engine/transaction.h"
#include "../../include/engine/engine.h"
#include "../../include/wal/wal.h"
#include <mutex>
#include <optional>>
#include <shared_mutex>

TranContext::TranContext(uint64_t tranc_id, std::shared_ptr<LSMEngine> engine,
                         std::shared_ptr<TranManager> manager, const enum IsolationLevel isolation_level)
    : tranc_id_(tranc_id), engine_(engine), manager_(manager)
{
    operations_.emplace_back(Record::createRecord(tranc_id_));
}

void TranContext::put(const std::string &key, const std::string &value)
{
    // 无论是什么隔离级别，都需要先插入操作记录到opertional_
    // operations_会输入WAL日志文件
    operations_.emplace_back(Record::putRecord(tranc_id_, key, value));

    if (isolation_level_ == IsolationLevel::ReadUncommitted)
    {
        // 如果隔离级别为ReadUncommitted：
        if (rollback_map_.find(key) == rollback_map_.end())
        {
            // 如果已经存在，就表示之前已经记录了这一个key的事务操作前的状态
            // 如果不存在，则需要记录这一个key的事务操作前的状态
            auto prev_record = engine_->get(key, tranc_id_);
        }
        engine_->put(key, value, tranc_id_);
        return;
    }
    // 其他隔离级别需要将数据暂存早当前上下文，以实现隔离性
    temp_map_[key] = value;
}

void TranContext::remove(const std::string &key) // remove与put操作一样
{
    // 无论什么隔离级别, 都需要先插入操作记录到 operations_
    // operations_ 会刷入 WAL 日志文件
    operations_.push_back(Record::deleteRecord(tranc_id_, key));

    if (isolation_level_ == IsolationLevel::ReadUncommitted)
    {
        if (rollback_map_.find(key) == rollback_map_.end())
        {
            // 如果已经存在, 就表示, 之前已经记录了这一个`key`的事务操作前的状态
            // 如果不存在, 则需要记录这一个`key`的事务操作前的状态
            auto prev_record = engine_->get(key, tranc_id_);
            rollback_map_[key] = prev_record;
        }
        engine_->remove(key, tranc_id_);
        return;
    }

    // 其他隔离级别需要将数据暂存到当前上下文, 以实现隔离性
    temp_map_[key] = "";
}

std::optional<std::string> TranContext::get(const std::string &key)
{
    // 1. 先从临时缓存 temp_map_ 中查找
    // 虽然 ReadUncommitted 隔离级别的 temp_map_ 是空的
    if (temp_map_.find(key) != temp_map_.end())
    {
        return temp_map_[key];
    }

    // 2. 如果临时缓存中没有, 再从数据库中查询
    std::optional<std::pair<std::string, uint64_t>> query;
    if (isolation_level_ == IsolationLevel::ReadUncommitted)
    {
        //  2.1 ReadUncommitted, 直接从存储引擎中查询
        query = engine_->get(key, 0);
    }
    else if (isolation_level_ == IsolationLevel::ReadCommitted)
    {
        // 2.2 ReadCommitted, 从存储引擎中查询, 并且需要考虑事务隔离性
        query = engine_->get(key, tranc_id_);
    }
    else
    {
        // 2.3 RepeatableRead 或 Serializable, 从存储引擎中查询,
        // 并且需要考虑事务隔离性
        /// current: 100 get kk -> yy
        /// other: 99 put kk -> xx
        /// current: 100 get kk -> yy

        if (read_map_.find(key) != read_map_.end())
        {
            query = read_map_[key]; // ？
        }
        else
        {
            query = engine_->get(key, tranc_id_);
            read_map_[key] = query;
        }
    }

    if (query.has_value())
    {
        return query.value().first;
    }
    return std::nullopt;
}

bool TranContext::commit()
{
    if (isolation_level_ == IsolationLevel::ReadUncommitted)
    {
        //  ReadUncommitted 每次操作都应用到了数据库, 不需要在 commit 时统一操作
        operations_.emplace_back(Record::commitRecord(tranc_id_));

        auto wal_success = manager_->write_to_wal(operations_);

        if(!wal_success)
        {
            throw std::runtime_error("Failed to write to WAL");
        }

        manager_->update_max_finished_tranc_id(tranc_id_);

        return true;
    }

    Memtable &memtable = engine_->memtable;                          // 获取存储引擎的内存引用
    std::unique_lock<std::shared_mutex> wlock1(memtable.frozen_mtx); // 加锁保护冻结内存表
    std::unique_lock<std::shared_mutex> wlock2(memtable.cur_mtx);    // 加锁保护当前内存表

    if (isolation_level_ == IsolationLevel::RepeatableRead ||
        isolation_level_ == IsolationLevel::Serializable)
    {
        // 判断 key 是否有冲突修改
        std::shared_lock<std::shared_mutex> rlock(this->engine_->ssts_mtx);

        for (auto &[k, v] : temp_map_) // 遍历事务暂存数据
        {
            auto res = memtable.get_(k, 0);                       // 从内存表查询键值
            if (res.is_valid() && res.get_tranc_id() > tranc_id_) // 内存表是否存在更高事务ID的修改
            {
                operations_.emplace_back(Record::rollbackRecord(tranc_id_)); // 添加回滚记录到操作列表
                // TODO: 需要刷入WAL的回滚标记（异步刷入的情况）

                return false; // 返回提交失败，触发回滚
            }
            else
            {
                if (tranc_id_ >= manager_->get_max_flushed_tranc_id()) // 检查事务ID是否大于最大刷盘ID
                {
                    continue; // 跳过SSTable检查
                }

                auto res = engine_->sst_get_(k, 0); // 从SSTable中查询键值
                if (res.has_value())                // 检查SSTable是否存在该键
                {
                    auto [v, tranc_id] = res.value(); // 结构查询结果
                    if (tranc_id > this->tranc_id_)   // 检查SSTable是否存在更高事务ID的修改
                    {
                        operations_.emplace_back(Record::rollbackRecord(tranc_id_));
                        // TODO: 需要刷入WAL的回滚标记（异步刷入的情况）

                        return false; // 返回提交失败，触发回滚
                    }
                }
            }
        }
    }

    operations_.emplace_back(Record::commitRecord(tranc_id_));

    auto wal_success = manager_->write_to_wal(operations_);
    if(!wal_success)
    {
        throw std::runtime_error("Failed to write to WAL");
    }

    manager_->update_max_finished_tranc_id(this->tranc_id_);

    return true;
}

bool TranContext::abort() // 事务的终止功能
{
    // 当隔离级别为ReadUncommitted时，遍历rollback_map_，将每个键值对恢复到事务开始前的状态。
    // 如果键值对存在，则将其写回引擎；如果不存在，则从引擎中移除该键。
    if (isolation_level_ == IsolationLevel::ReadUncommitted)
    {
        for (auto &[k, res] : rollback_map_)
        {
            if (res.has_value())
            {
                engine_->put(k, res.value().first, res.value().second);
            }
            else
            {
                engine_->remove(k, tranc_id_);
            }
        }
    }

    return true;
}

TranManager::TranManager(std::string data_dir,
                         enum IsolationLevel isolation_level)
{
    auto tranc_id_file_path = get_tranc_id_file_path();
    // 判断文件是否存在
    if (!std::filesystem::exists(tranc_id_file_path))
    {
        tranc_id_file_ = FileObj::open(tranc_id_file_path, true);
    }
    else
    {
        tranc_id_file_ = FileObj::open(tranc_id_file_path, false);
        read_tranc_id_file();
    }
}

std::string TranManager::get_tranc_id_file_path()
{
    if (!data_dir_.empty())
    {
        data_dir_ = "./";
    }
    return data_dir_ + "/tranc_id";
}

void TranManager::read_tranc_id_file()
{
    next_tranc_id_ = tranc_id_file_.read_uint64(0);
    max_flushed_tranc_id_ = tranc_id_file_.read_uint64(sizeof(next_tranc_id_));
    max_finished_tranc_id_ = tranc_id_file_.read_uint64(
        sizeof(next_tranc_id_) + sizeof(max_flushed_tranc_id_));
}

void TranManager::write_tranc_id_file()
{
    tranc_id_file_.write_uint64(0, next_tranc_id_);
    tranc_id_file_.write_uint64(sizeof(next_tranc_id_), max_flushed_tranc_id_);
    tranc_id_file_.write_uint64(sizeof(next_tranc_id_) +
                                    sizeof(max_flushed_tranc_id_),
                                max_finished_tranc_id_);
}

uint64_t TranManager::get_next_tranc_id()
{
    // TODO: 修改成原子操作的形式
    return next_tranc_id_++;
}

// 获取下一个事务ID，当前实现通过自增变量next_tranc_id_来生成新的ID。
std::shared_ptr<TranContext> TranManager::new_tranc(const enum IsolationLevel &isolation_level)
{
    // 获取锁
    std::unique_lock<std::mutex> lock(mutex_);

    auto tranc_id = get_next_tranc_id();
    auto context =
        std::make_shared<TranContext>(tranc_id, engine_, shared_from_this(), isolation_level);
    return context;
}

uint64_t TranManager::get_max_flushed_tranc_id()
{
    return max_flushed_tranc_id_;
}

std::map<uint64_t, std::vector<Record>> TranManager::check_recover() 
{
    std::map<uint64_t, std::vector<Record>> wal_records =
        Wal::recover(data_dir_, max_flushed_tranc_id_);
    return wal_records;
}
  
void TranManager::init_new_wal() {
    // 先删除旧的wal文件
    for (auto &entry : std::filesystem::directory_iterator(data_dir_)) {
        if (!entry.is_regular_file()) {
        continue;
        }

        std::string filename = entry.path().filename().string();
        if (filename.substr(0, 4) != "wal.") {
        continue;
        }

        std::filesystem::remove(entry.path());
    }

    wal_ = std::make_shared<Wal>(data_dir_, 128, max_finished_tranc_id_, 4096, 1);
}
  
bool TranManager::write_to_wal(std::vector<Record> operations) 
{
    try {
        wal_->log(operations);
    } catch (const std::exception &e) {
        return false;
    }
    return true;
}
  
void TranManager::update_max_flushed_tranc_id(uint64_t max_flushed_tranc_id) 
{
    // TODO: 修改成原子操作的形式
    std::unique_lock<std::mutex> lock(mutex_);
    max_flushed_tranc_id_ = std::max(max_flushed_tranc_id, max_flushed_tranc_id_);
}
void TranManager::update_max_finished_tranc_id(uint64_t max_finished_tranc_id) {
    // TODO: 修改成原子操作的形式
    std::unique_lock<std::mutex> lock(mutex_);
    max_finished_tranc_id_ =
        std::max(max_finished_tranc_id, max_finished_tranc_id_);
}