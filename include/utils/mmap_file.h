#pragma once

#include <string>
#include <vector>
#include <cstdint>

class MmapFile
{
public:
    MmapFile() : fd_(-1), mapped_data_(nullptr), file_size_(0) {}
    ~MmapFile() { close(); }
    void close();

    // 打开文件并映射到内存
    bool open(const std::string &filename, bool create = false);

    // 创建文件
    bool create(const std::string &filename, std::vector<uint8_t> &buf);

    // 获取文件大小
    size_t size() const { return file_size_; }

    // 写入数据
    bool write(size_t offset, const void *buf, size_t size);

    // 读取数据
    std::vector<uint8_t> read(size_t offset, size_t size);

    // 同步
    bool sync();

private:
    int fd_;
    void *mapped_data_; // 映射的地址
    size_t file_size_;
    std::string file_name_; // 文件名

    // 保证一个对象在操作数据
    MmapFile(const MmapFile &) = delete;
    MmapFile &operator=(const MmapFile &) = delete;

    // 获取映射内存的指针
    void *data() const { return mapped_data_; };

    // 创建文件并映射到内存
    bool create_and_map(const std::string &path, size_t size);
};