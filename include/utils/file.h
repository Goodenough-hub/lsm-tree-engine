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
    static FileObj open(const std::string &filename);

    // 读取文件
    std::vector<uint8_t> read_to_slice(size_t offset, size_t len);
};