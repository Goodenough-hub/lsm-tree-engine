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

FileObj FileObj::open(const std::string &path, bool create)
{
    FileObj file_obj;
    if (!file_obj.m_file->open(path, create))
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

bool FileObj::write(uint64_t offset, const std::vector<uint8_t> &buf)
{
    return m_file->write(offset, buf.data(), buf.size());
}

bool FileObj::write_uint64(uint64_t offset, const uint64_t &val)
{
    return m_file->write(offset, &val, sizeof(val));
}

bool FileObj::write_uint32(uint64_t offset, const uint32_t &val)
{
    return m_file->write(offset, &val, sizeof(val));
}

bool FileObj::write_uint16(uint64_t offset, const uint16_t &val)
{
    return m_file->write(offset, &val, sizeof(val));
}

uint64_t FileObj::read_uint64(uint64_t offset)
{
    // 边界检查
    if (offset + sizeof(uint64_t) > size())
    {
        throw std::runtime_error("read out of range");
    }

    auto result = m_file->read(offset, sizeof(uint64_t));
    return *reinterpret_cast<uint64_t *>(result.data());
}

uint32_t FileObj::read_uint32(uint64_t offset)
{
    // 边界检查
    if (offset + sizeof(uint32_t) > size())
    {
        throw std::runtime_error("read out of range");
    }

    auto result = m_file->read(offset, sizeof(uint32_t));
    return *reinterpret_cast<uint32_t *>(result.data());
}

uint16_t FileObj::read_uint16(uint64_t offset)
{
    // 边界检查
    if (offset + sizeof(uint16_t) > size())
    {
        throw std::runtime_error("read out of range");
    }

    auto result = m_file->read(offset, sizeof(uint16_t));
    return *reinterpret_cast<uint16_t *>(result.data());
}

// 将给定的字节缓冲区追加到文件末尾
bool FileObj::append(std::vector<uint8_t> buf)
{
    auto file_size = m_file->size();

    if(!m_file->write(file_size, buf.data(), buf.size()))
    {
        return false;
    }
    
    m_size += buf.size();
    return true;
}

bool FileObj::sync()
{
    return m_file->sync();
}

void  FileObj::del_file()
{
    m_file->remove();
}