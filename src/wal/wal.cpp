#include "../../include/wal/wal.h"
#include <algorithm>
#include <mutex>
#include <string>

Wal::Wal(const std::string &log_dir, size_t buffer_size,
         uint64_t max_finished_tranc_id, size_t file_size_limit,
         uint64_t clean_interval)
    : buffer_size_(buffer_size), file_size_limit_(file_size_limit),
      max_finished_tranc_id_(max_finished_tranc_id), stop_clenner_(false),
      clean_interval_(1000) {
  active_log_path_ = log_dir + "/wal.0";
  log_file_ = FileObj::open(active_log_path_, true);

  //  TODO: 清理线程
}

Wal::~Wal() {
  // TODO
}

void Wal::log(const std::vector<Record> &records, bool force_flush) {
  std::unique_lock<std::mutex> lock(mutex_);

  // 添加到缓冲区中
  for (const auto &record : records) {
    buffer_.push_back(record);
  }

  if (buffer_.size() < buffer_size_ || !force_flush) {
    return;
  }

  auto prev_buffer = std::move(buffer_);
  for (const auto &record : prev_buffer) {
    auto encoded_data = record.encode();
    log_file_.append(encoded_data);
  }

  if (!log_file_.sync()) {
    throw std::runtime_error("wal sync error");
  }

  auto cur_wal_size = log_file_.size();
  if (cur_wal_size > file_size_limit_) {
    reset_file_();
  }
}

void Wal::reset_file_() {
  auto old_path = active_log_path_;
  auto seq = std::stoi(old_path.substr(old_path.find_last_of(".") + 1));
  seq++;

  active_log_path_ = old_path.substr(0, old_path.find_last_of(".")) + "." +
                     std::to_string(seq);

  log_file_ = FileObj::open(active_log_path_, true);
}

void Wal::flush() {
  //  TODO: 后续异步刷盘时使用
}

void Wal::set_max_finished_tranc_id(uint64_t max_finished_tranc_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  max_finished_tranc_id_ = max_finished_tranc_id;
}