#include "../include/utils/file.h"
#include <filesystem>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#define TEST_DIR "test_dir"
#define TEST_FILE TEST_DIR "/text.txt"

class FileTest : public ::testing::Test
{
protected:
    void SetUp() override // 每个测试用例前执行
    {
        std::filesystem::remove_all(TEST_DIR);
        std::filesystem::create_directory(TEST_DIR);
    }

    void TearDown() override // 每个测试用例后执行
    {
        std::filesystem::remove_all(TEST_DIR);
    }
};

TEST_F(FileTest, BasicWriteAndRead)
{
    std::vector<uint8_t> buf = {1, 2, 3, 4, 5};
    FileObj file = FileObj::create_and_write(TEST_FILE, buf);

    auto file_read = FileObj::open(TEST_FILE, true);
    auto read_buf = file_read.read_to_slice(1, 2);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}