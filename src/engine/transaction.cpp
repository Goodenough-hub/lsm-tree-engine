#include "../../include/engine/transaction.h"

TranContext::TranContext(uint64_t tranc_id, std::shared_ptr<LSMEngine> engine,
                         std::shared_ptr<TranManager> manager)
    : tranc_id_(tranc_id), engine_(engine), manager_(manager) {}

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
std::shared_ptr<TranContext> TranManager::new_tranc()
{
    // 获取锁
    std::unique_lock<std::mutex> lock(mutex_);

    auto tranc_id = get_next_tranc_id();
    auto context =
        std::make_shared<TranContext>(tranc_id, engine_, shared_from_this());
    return context;
}