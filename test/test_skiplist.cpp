#include "../include/skiplist/skiplist.h"
#include <gtest/gtest.h>

TEST(SkipListTest, BasicOperations) {
  SkipList skiplist;

  skiplist.put("key1", "value1");
  EXPECT_EQ(skiplist.get("key1").value(), "value1");

  skiplist.put("key1", "value2");
  EXPECT_EQ(skiplist.get("key1").value(), "value2");

  skiplist.remove("key1");
  EXPECT_FALSE(skiplist.get("key1").has_value());
}

TEST(SkipListTest, LargeScaleSetAndGet) {
  SkipList skiplist;

  const int num_elements = 100000;

  for (int i = 0; i < num_elements; i++) {
    std::string key = "key" + std::to_string(i);
    std::string value = "value" + std::to_string(i);
    skiplist.put(key, value);
  }

  for (int i = 0; i < num_elements; i++) {
    std::string key = "key" + std::to_string(i);
    std::string value = "value" + std::to_string(i);
    auto res = skiplist.get(key);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), value);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}