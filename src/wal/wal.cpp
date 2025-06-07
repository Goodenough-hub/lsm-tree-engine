#include "../../include/wal/wal.h"
#include <algorithm>
#include <mutex>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

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

// 从指定目录中恢复wal日志文件，并解析其中的记录。
// 其中包括：检查日志是否存在，遍历目录中的WAL文件，按文件序号排序，读取并解析文件内容，筛选出事务ID大于指定最大值的记录
// 返回值是：按事务ID分组的记录
std::map<uint64_t, std::vector<Record>>
Wal::recover(const std::string &log_dir, uint64_t max_flashed_tranc_id) {
  std::map<uint64_t, std::vector<Record>> wal_records;

  if (!std::filesystem::exists(log_dir)) {
    return wal_records;
  }

  // 遍历wal文件, 解析
  std::vector<std::string> wal_paths;

  for (auto &entry : std::filesystem::directory_iterator(log_dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }

    std::string filename = entry.path().filename().string();
    if (filename.substr(0, 4) != "wal.") {
      continue;
    }

    wal_paths.push_back(entry.path());
  }

  std::sort(wal_paths.begin(), wal_paths.end(),
            [](const std::string &a, const std::string &b) {
              auto a_seq_str = a.substr(a.find_last_of(".") + 1);
              auto b_seq_str = b.substr(b.find_last_of(".") + 1);
              return std::stoi(a_seq_str) < std::stoi(b_seq_str);
            });

  // 读取所有的记录
  for (const auto &wal_path : wal_paths) {
    FileObj wal_file = FileObj::open(wal_path, false);
    auto wal_file_slice = wal_file.read_to_slice(0, wal_file.size());
    auto records = Record::decode(wal_file_slice);

    for (const auto &record : records) {
      if (record.tranc_id_ > max_flashed_tranc_id) {
        wal_records[record.tranc_id_].push_back(record);
      }
    }
  }

  return wal_records;
}