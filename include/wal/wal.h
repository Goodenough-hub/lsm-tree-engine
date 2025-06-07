#pragma once

#include "../utils/file.h"
#include "record.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// wal 文件格式:
// wal.0 wal.1 wal.2
class Wal {

public:
 // 初始化WAL实例，指定日志目录，缓冲区大小，最大已经完成事务ID，文件大小限制和清理间隔
  Wal(const std::string &log_dir, size_t buffer_size,
      uint64_t max_finished_tranc_id, size_t file_size_limit,
      uint64_t clean_interval);

  ~Wal(); // 清理资源，停止后台清理线程

  // 从日志目录中恢复日志数据，返回一个映射（map），其中键为事务 ID，值为对应的记录列表。
  static std::map<uint64_t, std::vector<Record>>
  recover(const std::string &log_dir, uint64_t max_finished_tranc_id);

  // 将一组记录（Record）写入日志缓冲区，支持强制刷新到文件。
  void log(const std::vector<Record> &records, bool force_flush = false);

  // 将缓冲区中的数据强制刷新到日志文件。
  void flush();

  // 设置最大已完成事务 ID，用于清理过期的日志文件。
  void set_max_finished_tranc_id(uint64_t max_finished_tranc_id);

private: 
  // 置当前日志文件，通常在文件达到大小限制时调用。
  void reset_file_();

protected:
  std::string active_log_path_;
  FileObj log_file_;
  size_t file_size_limit_;
  std::mutex mutex_;
  size_t buffer_size_;
  std::vector<Record> buffer_;
  std::thread clean_thread_;
  uint64_t max_finished_tranc_id_ = 0;
  std::atomic<bool> stop_clenner_;
  uint64_t clean_interval_;
};