#pragma once

#include "../utils/file.h"
#include "../wal/record.h"
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include <unordered_map>
#include <optional>

enum class IsolationLevel // 定义隔离级别
{
    ReadUncommitted, // 读未提交
    ReadCommitted,   // 读已提交
    RepeatableRead,  // 可重复读
    Serializable,    // 可串行化
};

class LSMEngine;
class TranManager;

// 定义事务的上下文，封装了事务操作记录和状态
class TranContext
{
    friend class TranManager;

private:
    uint64_t tranc_id_;
    std::shared_ptr<LSMEngine> engine_;
    std::shared_ptr<TranManager> manager_; // 指向事务管理器的指针，用于管理事务生命周期
    std::vector<Record> operations_;       // 存储事务的所有操作记录
    enum IsolationLevel isolation_level_;

    // 这三个unordered_map是事务上下文类TranContext的核心数据结构，分别用于事务的回滚控制、一致性读和临时写操作。
    //
    // 用于记录事务中被修改或删除的键值对的原始值（即回滚信息）。键为数据的键，值为原始值及其版本号（或时间戳），以便在事务回滚时恢复到修改前的状态。
    // 记录首次修改的原始值
    std::unordered_map<std::string, std::optional<std::pair<std::string, uint64_t>>> rollback_map_;
    // 用于记录事务中读取的键值对及其版本号（或时间戳）。这有助于在不同隔离级别下（如可重复读、可串行化）确保读取的一致性，避免幻读或脏读等问题。
    // 根据事务的隔离级别记录，可重复读和可串行化需要记录读取的键值对及其版本号，而读未提交和读已提交则不需要。
    // 在 get 操作时记录读取的键值及其版本号
    std::unordered_map<std::string, std::optional<std::pair<std::string, uint64_t>>> read_map_;
    // 用于临时存储事务中未提交的写操作结果。键为数据的键，值为对应的临时值。该映射仅在事务提交时才会将数据持久化到存储引擎，或者在事务回滚时丢弃。
    // 暂存未提交的写操作
    std::unordered_map<std::string, std::optional<std::string>> temp_map_;

public:
    TranContext(uint64_t tranc_id, std::shared_ptr<LSMEngine> engine,
                std::shared_ptr<TranManager> manager,
                const enum IsolationLevel isolation_level);
    void put(const std::string &key, const std::string &value);
    void remove(const std::string &key);
    std::optional<std::string> get(const std::string &key);
    bool commit();
    bool abort();
};

// 事务管理器，负责创建、管理和恢复事务
class TranManager : public std::enable_shared_from_this<TranManager>
{
public:
    TranManager(std::string data_dir, enum IsolationLevel isolation_level);
    ~TranManager();
    // TODO 管理 wal
    void set_engine(std::shared_ptr<LSMEngine> engine);
    std::shared_ptr<TranContext> new_tranc(const enum IsolationLevel &isolation_level);

    // TODO 启动前崩溃恢复的检查
    std::map<uint64_t, std::vector<Record>> check_recover();

    uint64_t get_max_flushed_tranc_id();

private:
    std::string get_tranc_id_file_path();
    void read_tranc_id_file();
    void write_tranc_id_file();
    uint64_t get_next_tranc_id();

private:
    mutable std::mutex mutex_;
    std::shared_ptr<LSMEngine> engine_;
    // TODO: WAL相关类
    std::string data_dir_; // 数据目录路径，用于存储事务相关文件
    // enum IsolationLevel isolation_level_; // 事务隔离级别
    // TODO: 改成原子变量
    uint64_t next_tranc_id_ = 1;         // 下一个可用的事务ID
    uint64_t max_flushed_tranc_id_ = 0;  // 已刷盘的最大事务ID。事务是否已经持久化
    uint64_t max_finished_tranc_id_ = 0; // 已完成的最大事务ID。事务是否已经成功提交或回滚
    FileObj tranc_id_file_;              // 事务ID文件,用于持久化事务ID
};