#pragma once

#include "mmap_file.h"
#include <memory>
#include <vector>
#include <cstring>

class FileObj
{
private:
    std::unique_ptr<MmapFile> m_file;
    size_t m_size;

public:
    FileObj();
    ~FileObj();

    FileObj(const FileObj &) = delete;
    FileObj &operator=(const FileObj &) = delete;

    // 实现移动语义，noexcept要求不会抛出异常
    FileObj(FileObj &&) noexcept;
    FileObj &operator=(FileObj &&) noexcept;

    // 文件大小
    size_t size() const;

    // 设置文件大小
    void set_size(size_t size);

    // 设置文件对象并写入磁盘
    static FileObj create_and_write(const std::string &filename, std::vector<uint8_t> &buf);

    // 打开文件对象
    static FileObj open(const std::string &filename, bool create);

    // 读取文件
    std::vector<uint8_t> read_to_slice(size_t offset, size_t len);

    bool write(uint64_t offset, const std::vector<uint8_t> &buf);
    bool write_uint64(uint64_t offset, const uint64_t &val);
    bool write_uint32(uint64_t offset, const uint32_t &val);
    bool write_uint16(uint64_t offset, const uint16_t &val);

    uint64_t read_uint64(uint64_t offset);
    uint32_t read_uint32(uint64_t offset);
    uint16_t read_uint16(uint64_t offset);
};