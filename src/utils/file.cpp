#include "../../include/utils/file.h"

FileObj::FileObj() : m_file(std::make_unique<MmapFile>()) {}

FileObj::~FileObj() = default;

// 实现移动语义
FileObj::FileObj(FileObj &&other) noexcept
    : m_file(std::move(other.m_file)), m_size(other.m_size)
{
    other.m_size = 0;
}

FileObj &FileObj::operator=(FileObj &&other) noexcept
{
    if (this != &other)
    {
        m_file = std::move(other.m_file);
        m_size = other.m_size;
        other.m_size = 0;
    }
    return *this;
}

size_t FileObj::size() const
{
    return m_file->size();
}

void FileObj::set_size(size_t size)
{
    m_size = size;
}

// 创建文件对象，并写入磁盘
FileObj FileObj::create_and_write(const std::string &filename, std::vector<uint8_t> &buf)
{
    FileObj file_obj;
    if (!file_obj.m_file->create(filename, buf))
    {
        throw std::runtime_error("create file failed");
    }

    // 同步到磁盘
    file_obj.m_file->sync();

    return std::move(file_obj);
}

FileObj FileObj::open(const std::string &path)
{
    FileObj file_obj;
    if (!file_obj.m_file->open(path, false))
    {
        throw std::runtime_error("open file failed");
    }

    return std::move(file_obj);
}

std::vector<uint8_t> FileObj::read_to_slice(size_t offset, size_t len)
{
    if (offset + len > m_size)
    {
        throw std::runtime_error("read file failed");
    }

    auto res = m_file->read(offset, len);

    return res;
}