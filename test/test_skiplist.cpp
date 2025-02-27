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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}