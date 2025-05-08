#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

class StdFile
{
private:
    std::fstream file_;
    std::filesystem::path file_name_;

public:
    StdFile() {}

    ~StdFile()
    {
        if (file_.is_open())
        {
            file_.close();
        }
    }

    bool open(const std::string &file_name, bool create);

    bool create(const std::string &file_name, std::vector<uint8_t> &buf);

    void close();

    size_t size();

    bool write(size_t offset, const void *data, size_t size);

    std::vector<uint8_t> read(size_t offset, size_t size);

    bool sync(); // 同步
    
    bool remove();
};