#include "../include/memtable/memtable.h"
#include <gtest/gtest.h>

TEST(MemtableTest, BasicOperations)
{
  Memtable memtable;

  memtable.put("key1", "value1");
  EXPECT_EQ(memtable.get("key1").value(), "value1");

  memtable.put("key1", "value2");
  EXPECT_EQ(memtable.get("key1").value(), "value2");

  memtable.remove("key1");
  EXPECT_EQ(memtable.get("key1").value(), "");
}

TEST(MemTableTest, FrozenTableOperations)
{
  Memtable memtable;

  memtable.put("key1", "value1");
  memtable.put("key2", "value2");

  memtable.frozen_cur_table();
  memtable.put("key3", "value3");
  memtable.put("key2", "value22");

  auto res = memtable.get("key1").value();
  EXPECT_EQ(res, "value1");
  res = memtable.get("key2").value();
  EXPECT_EQ(memtable.get("key2").value(), "value22");
  res = memtable.get("key3").value();
  EXPECT_EQ(memtable.get("key3").value(), "value3");
}

TEST(MemTableTest, LargeScaleOperations)
{
  Memtable memtable;

  const int num_entries = 10000;
  for (int i = 0; i < num_entries; i++)
  {
    std::string key = "key" + std::to_string(i);
    std::string value = "value" + std::to_string(i);
    memtable.put(key, value);
  }

  for (int i = 0; i < num_entries; i++)
  {
    std::string key = "key" + std::to_string(i);
    std::string value = "value" + std::to_string(i);
    EXPECT_EQ(memtable.get(key).value(), value);
  }
}

TEST(MemTableTest, IteratorComplexOperations)
{
  Memtable memtable;

  memtable.put("key1", "value1");
  memtable.put("key2", "value2");
  memtable.put("key3", "value3");

  std::vector<std::pair<std::string, std::string>> results;
  for (auto it = memtable.begin(); it != memtable.end(); ++it)
  {
    results.push_back(*it);
  }

  ASSERT_EQ(results.size(), 3);
  EXPECT_EQ(results[0].first, "key1");
  EXPECT_EQ(results[0].second, "value1");
  EXPECT_EQ(results[2].first, "key3");

  memtable.frozen_cur_table();

  memtable.put("key2", "value2_updated");
  memtable.remove("key1");
  memtable.put("key4", "value4");

  std::vector<std::pair<std::string, std::string>> results2;
  for (auto it = memtable.begin(); it != memtable.end(); ++it)
  {
    results2.push_back(*it);
  }

  ASSERT_EQ(results2.size(), 3);
  EXPECT_EQ(results2[0].first, "key2");
  EXPECT_EQ(results2[0].second, "value2_updated");
  EXPECT_EQ(results2[1].first, "key3");
  EXPECT_EQ(results2[2].first, "key4");
}

TEST(MemTableTest, ConcurrentOperations)
{
  Memtable memtable;
  const int num_readers = 4;       // 读线程数
  const int num_writers = 2;       // 写线程数
  const int num_operations = 1000; // 每个线程的操作数

  // 用于同步所有线程的开始
  std::atomic<bool> start{false};
  // 用于等待所有线程完成
  std::atomic<int> completion_counter{num_readers + num_writers +
                                      1}; // +1 for freeze thread

  // 记录写入的键，用于验证
  std::vector<std::string> inserted_keys;
  std::mutex keys_mutex;

  // 写线程函数
  auto writer_func = [&](int thread_id)
  {
    while (!start)
    {
      std::this_thread::yield();
    }

    for (int i = 0; i < num_operations; ++i)
    {
      std::string key =
          "key_" + std::to_string(thread_id) + "_" + std::to_string(i);
      std::string value =
          "value_" + std::to_string(thread_id) + "_" + std::to_string(i);

      if (i % 3 == 0)
      {
        // 插入操作
        memtable.put(key, value);
        {
          std::lock_guard<std::mutex> lock(keys_mutex);
          inserted_keys.push_back(key);
        }
      }
      else if (i % 3 == 1)
      {
        // 删除操作
        memtable.remove(key);
      }
      else
      {
        // 更新操作
        memtable.put(key, value + "_updated");
      }

      std::this_thread::sleep_for(std::chrono::microseconds(rand() % 100));
    }

    completion_counter--;
  };

  // 读线程函数
  auto reader_func = [&](int thread_id)
  {
    while (!start)
    {
      std::this_thread::yield();
    }

    int found_count = 0;
    for (int i = 0; i < num_operations; ++i)
    {
      // 随机选择一个已插入的key进行查询
      std::string key_to_find;
      {
        std::lock_guard<std::mutex> lock(keys_mutex);
        if (!inserted_keys.empty())
        {
          key_to_find = inserted_keys[rand() % inserted_keys.size()];
        }
      }

      if (!key_to_find.empty())
      {
        auto result = memtable.get(key_to_find);
        if (result.has_value())
        {
          found_count++;
        }
      }

      // 每隔一段时间进行一次遍历操作
      if (i % 100 == 0)
      {
        std::vector<std::pair<std::string, std::string>> items;
        for (auto it = memtable.begin(); it != memtable.end(); ++it)
        {
          items.push_back(*it);
        }
      }

      std::this_thread::sleep_for(std::chrono::microseconds(rand() % 50));
    }

    completion_counter--;
  };

  // 冻结线程函数
  auto freeze_func = [&]()
  {
    while (!start)
    {
      std::this_thread::yield();
    }

    // 定期执行冻结操作
    for (int i = 0; i < 5; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      memtable.frozen_cur_table();

      // 验证冻结后的表
      size_t frozen_size = memtable.get_frozen_size();
      EXPECT_GE(frozen_size, 0);

      // 验证总大小
      size_t total_size = memtable.get_total_size();
      EXPECT_GE(total_size, frozen_size);
    }

    completion_counter--;
  };

  // 创建并启动写线程
  std::vector<std::thread> writers;
  for (int i = 0; i < num_writers; ++i)
  {
    writers.emplace_back(writer_func, i);
  }

  // 创建并启动读线程
  std::vector<std::thread> readers;
  for (int i = 0; i < num_readers; ++i)
  {
    readers.emplace_back(reader_func, i);
  }

  // 创建并启动冻结线程
  std::thread freeze_thread(freeze_func);

  // 给线程一点时间进入等待状态
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // 记录开始时间
  auto start_time = std::chrono::high_resolution_clock::now();

  // 发送开始信号
  start = true;

  // 等待所有线程完成
  while (completion_counter > 0)
  {
    std::this_thread::yield();
  }

  // 记录结束时间
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);

  // 等待所有线程结束
  for (auto &w : writers)
  {
    w.join();
  }
  for (auto &r : readers)
  {
    r.join();
  }
  freeze_thread.join();

  // 验证最终状态
  size_t final_size = 0;
  for (auto it = memtable.begin(); it != memtable.end(); ++it)
  {
    final_size++;
  }

  // 输出统计信息
  // std::cout << "Concurrent test completed in " << duration.count()
  //           << "ms\nFinal memtable size: " << final_size
  //           << "\nTotal size: " << memtable.get_total_size()
  //           << "\nFrozen size: " << memtable.get_frozen_size() << std::endl;

  // 基本正确性检查
  EXPECT_GT(memtable.get_total_size(), 0);             // 总大小应该大于0
  EXPECT_LE(final_size, num_writers * num_operations); // 大小不应超过最大可能值
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}