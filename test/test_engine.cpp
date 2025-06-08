#include "../include/engine/engine.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

class EngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir = "test_engine_data";
        if (std::filesystem::exists(test_dir))
        {
            std::filesystem::remove_all(test_dir);
        }
        std::filesystem::create_directory(test_dir);
    }

    void TearDown() override
    {
        if (std::filesystem::exists(test_dir))
        {
            std::filesystem::remove_all(test_dir);
        }
    }

    std::string test_dir;
};

TEST_F(EngineTest, TestEngine)
{
    LSMEngine engine(test_dir);

    engine.put("key1", "value1", 0);
    ASSERT_EQ(engine.get("key1", 0).value().first, "value1");

    engine.put("key1", "value11", 0);
    ASSERT_EQ(engine.get("key1", 0).value().first, "value11");

    engine.remove("key1", 0);
    ASSERT_FALSE(engine.get("key1", 0).has_value());
}

TEST_F(EngineTest, PersistenceTest)
{
    std::unordered_map<std::string, std::string> kvs;
    int num = 100000;

    {
        LSMEngine engine(test_dir);
        for (int i = 0; i < num; i++)
        {
            std::string key = "key" + std::to_string(i);
            std::string value = "value" + std::to_string(i);
            engine.put(key, value, 0);
            kvs[key] = value;

            if (i % 10 == 0 && i != 0)
            {
                std::string del_key = "key" + std::to_string(i - 10);
                engine.remove(del_key, 0);
                kvs.erase(del_key);
            }
        }
    }

    LSMEngine engine(test_dir);
    for (int i = 0; i < num; i++)
    {
        std::string key = "key" + std::to_string(i);
        if (kvs.find(key) != kvs.end())
        {
            ASSERT_EQ(engine.get(key, 0).value().first, kvs[key]);
        }
        else
        {
            ASSERT_FALSE(engine.get(key, 0).has_value());
        }
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}