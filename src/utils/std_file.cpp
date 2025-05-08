#include "../../include/utils/std_file.h"
#include <ios>

bool StdFile::open(const std::string &file_name, bool create)
{
    file_name_ = file_name;

    if (create)
    {
        file_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    }
    else
    {
        file_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary);
    }

    return file_.is_open(); // 检查文件流是否已经成功打开
}

bool StdFile::create(const std::string &file_name, std::vector<uint8_t> &buf)
{
    if (!this->open(file_name, true))
    {
        throw std::runtime_error("Failed to create file.");
    }
    write(0, buf.data(), buf.size());
    return true;
}

void StdFile::close()
{
    if (file_.is_open())
    {
        sync();
        file_.close();
    }
}

size_t StdFile::size()
{
    file_.seekg(0, std::ios::end); // 文件指针移动到文件末尾
    return file_.tellg();          // 获取文件指针当前位置，即文件大小
}

bool StdFile::write(size_t offset, const void *data, size_t length)
{
    file_.seekg(offset, std::ios::beg);
    file_.write(static_cast<const char *>(data), length);
    sync();
    return true;
}

std::vector<uint8_t> StdFile::read(size_t offset, size_t length)
{
    std::vector<uint8_t> buf(length);
    file_.seekg(offset, std::ios::beg);
    if (!file_.read(reinterpret_cast<char *>(buf.data()), length))
    {
        throw std::runtime_error("Failed to read file.");
    }
    return buf;
}

bool StdFile::sync()
{
    if (!file_.is_open())
    {
        return false;
    }
    file_.flush();       // 强制将缓冲区的数据写入到文件中，并清空缓冲区。
    return file_.good(); // 检查文件流是否处于正常状态，写入失败时返回false。
}

bool StdFile::remove()
{
    if(file_.is_open())
    {
        file_.close();
    }
    return std::filesystem::remove(file_name_);
}