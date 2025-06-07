#include "../../include/utils/file.h"
#include "../../include/utils/mmap_file.h"
#include <algorithm>
#include <cstdint>
#include <memory>

FileObj::FileObj() : m_file(std::make_unique<StdFile>()) {}

FileObj::~FileObj() = default;

// 实现移动语义
FileObj::FileObj(FileObj &&other) noexcept
    : m_file(std::move(other.m_file)), m_size(other.m_size) {
  other.m_size = 0;
}

FileObj &FileObj::operator=(FileObj &&other) noexcept {
  if (this != &other) {
    m_file = std::move(other.m_file);
    m_size = other.m_size;
    other.m_size = 0;
  }
  return *this;
}

size_t FileObj::size() const { return m_file->size(); }

void FileObj::set_size(size_t size) { m_size = size; }

FileObj FileObj::create_and_write(const std::string &path,
                                  std::vector<uint8_t> buf) {
  FileObj file_obj;

  if (!file_obj.m_file->create(path, buf)) {
    throw std::runtime_error("Failed to create file");
  }

  // 同步到磁盘
  file_obj.m_file->sync();

  return std::move(file_obj);
}

FileObj FileObj::open(const std::string &path, bool create) {
  FileObj file_obj;

  // 打开
  if (!file_obj.m_file->open(path, create)) {
    throw std::runtime_error("Failed to open file");
  }

  return std::move(file_obj);
}

std::vector<uint8_t> FileObj::read_to_slice(size_t offset, size_t length) {
  if (offset + length > m_file->size()) {
    throw std::runtime_error("Read out of range");
  }

  auto result = m_file->read(offset, length);

  return result;
}

bool FileObj::write(uint64_t offset, std::vector<uint8_t> &buf) {
  return m_file->write(offset, buf.data(), buf.size());
}

bool FileObj::write_uint64(uint64_t offset, const uint64_t &val) {
  return m_file->write(offset, &val, sizeof(val));
}

bool FileObj::write_uint32(uint64_t offset, const uint32_t &val) {
  return m_file->write(offset, &val, sizeof(val));
}

bool FileObj::write_uint16(uint64_t offset, const uint16_t &val) {
  return m_file->write(offset, &val, sizeof(val));
}

uint64_t FileObj::read_uint64(uint64_t offset) {
  // 边界检查
  if (offset + sizeof(uint64_t) > size()) {
    throw std::out_of_range("read out of range");
  }

  auto result = m_file->read(offset, sizeof(uint64_t));

  return *reinterpret_cast<uint64_t *>(result.data());
}
uint32_t FileObj::read_uint32(uint64_t offset) {
  // 边界检查
  if (offset + sizeof(uint32_t) > size()) {
    throw std::out_of_range("read out of range");
  }
  auto result = m_file->read(offset, sizeof(uint32_t));
  return *reinterpret_cast<uint32_t *>(result.data());
}
uint16_t FileObj::read_uint16(uint64_t offset) {
  // 边界检查
  if (offset + sizeof(uint16_t) > size()) {
    throw std::out_of_range("read out of range");
  }
  auto result = m_file->read(offset, sizeof(uint16_t));
  return *reinterpret_cast<uint16_t *>(result.data());
}

bool FileObj::append(std::vector<uint8_t>& buf) {
  auto file_size = m_file->size();

  if (!m_file->write(file_size, buf.data(), buf.size())) {
    return false;
  };

  m_size += buf.size();
  return true;
}

bool FileObj::sync() { return m_file->sync(); }

void FileObj::del_file() { m_file->remove(); }