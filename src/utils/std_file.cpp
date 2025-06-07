#include "../../include/utils/std_file.h"
#include <ios>
#include <vector>

bool StdFile::open(const std::string &file_name, bool create) {
  file_name_ = file_name;

  if (create) {
    file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary |
                              std::ios::trunc);
  } else {
    file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
  }

  return file_.is_open();
}

bool StdFile::create(const std::string &file_name, std::vector<uint8_t> &buf) {
  if (!this->open(file_name, true)) {
    throw std::runtime_error("Failed to create file");
  }
  write(0, buf.data(), buf.size());
  return true;
}

void StdFile::close() {
  if (file_.is_open()) {
    sync();
    file_.close();
  }
}

size_t StdFile::size() {
  file_.seekg(0, std::ios::end);
  return file_.tellg();
}

bool StdFile::write(size_t offset, const void *data, size_t length) {
  file_.seekg(offset, std::ios::beg);
  file_.write(static_cast<const char *>(data), length);
  sync();
  return true;
}

std::vector<uint8_t> StdFile::read(size_t offset, size_t length) {
  std::vector<uint8_t> buf(length);
  file_.seekg(offset, std::ios::beg);
  if (!file_.read(reinterpret_cast<char *>(buf.data()), length)) {
    throw std::runtime_error("Failed to read file");
  }
  return buf;
}

bool StdFile::sync() {
  if (!file_.is_open()) {
    return false;
  }
  file_.flush();
  return file_.good();
}

bool StdFile::remove() {
  if (file_.is_open()) {
    file_.close();
  }
  return std::filesystem::remove(file_name_);
}