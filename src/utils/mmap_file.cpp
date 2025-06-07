#include "../../include/utils/mmap_file.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

bool MmapFile::create_and_map(const std::string &path, size_t size)
{
    // 创建文件并映射到内存
    fd_ = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (fd_ == -1)
    {
        return false; // 表示请求失败
    }

    // 调节文件大小
    if (ftruncate(fd_, size) == -1)
    {
        close();
        return false;
    }

    // 映射与问价大小相同的内存
    mapped_data_ = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (mapped_data_ == MAP_FAILED)
    {
        close();
        return false;
    }

    file_size_ = size;
    return true;
}

bool MmapFile::open(const std::string &path, bool create)
{
    this->file_name_ = path;

    // 打开或创建文件
    int flag = O_RDWR;
    if (create)
    {
        flag |= O_CREAT;
    }

    fd_ = ::open(path.c_str(), flag, 0644);

    if (fd_ == -1)
    {
        return false;
    }

    // 获取文件大小
    struct stat st;
    if (fstat(fd_, &st) == -1)
    {
        close();
        return false;
    }

    file_size_ = st.st_size;

    // 映射文件
    if (file_size_ > 0)
    {
        mapped_data_ = mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mapped_data_ == MAP_FAILED)
        {
            close();
            return false;
        }
    }
    return true;
}

bool MmapFile::create(const std::string &filename, std::vector<uint8_t> &buf)
{
    if (!create_and_map(filename, buf.size()))
    {
        return false;
    }

    // 写入数据
    memcpy(this->data(), buf.data(), buf.size());

    this->sync();

    return true;
}

void MmapFile::close()
{
    if (mapped_data_ != nullptr && mapped_data_ != MAP_FAILED)
    {
        munmap(mapped_data_, file_size_);
        mapped_data_ = nullptr;
    }

    if (fd_ != -1)
    {
        ::close(fd_); // 表示调用系统的接口
    }
    file_size_ = 0;
}

bool MmapFile::write(size_t offset, const void *buf, size_t size)
{
    // 调整文件大小，要包含 offset + size
    size_t new_size = offset + size;
    if (ftruncate(fd_, new_size) == -1)
    {
        return false;
    }

    // 如果已经映射了，解除映射
    if (mapped_data_ != nullptr && mapped_data_ != MAP_FAILED)
    {
        munmap(mapped_data_, file_size_);
        mapped_data_ = nullptr;
    }

    // 重新映射
    file_size_ = new_size;
    mapped_data_ = mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);

    if (mapped_data_ == MAP_FAILED)
    {
        close();
        return false;
    }

    // 写入数据
    memcpy((uint8_t *)mapped_data_ + offset, buf, size);

    this->sync();

    return true;
}

std::vector<uint8_t> MmapFile::read(size_t offset, size_t size)
{
    std::vector<uint8_t> buf(size);
    memcpy(buf.data(), (uint8_t *)mapped_data_ + offset, size);
    return buf;
}

bool MmapFile::sync()
{
    if (mapped_data_ != nullptr && mapped_data_ != MAP_FAILED)
        return msync(mapped_data_, file_size_, MS_SYNC) == 0;
    return true;
}

bool MmapFile::remove()
{
    if(fd_ != -1)
    {
        close();
    }
    return ::remove(file_name_.c_str()) == 0;
}