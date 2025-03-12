#include "../include/block/block_cache.h"
#include <gtest/gtest.h>
#include <memory>

class BlockCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        cache = std::make_unique<BlockCache>(3, 2);
    }

    std::shared_ptr<BlockCache> cache;
};

TEST_F(BlockCacheTest, TestPutAndGet)
{
    auto block1 = std::make_shared<Block>();
    auto block2 = std::make_shared<Block>();
    auto block3 = std::make_shared<Block>();

    cache->put(1, 1, block1);
    cache->put(1, 2, block2);
    cache->put(1, 3, block1);

    EXPECT_EQ(cache->get(1, 1), block1)
    EXPECT_EQ(cache->get(1, 2), block2);
    EXCEPT_EQ(cache->get(1, 3), block3);

    auto block4 = std::make_shared<Block>();
    cache->put(1, 4, block4);
    EXPECT_EQ(cache->get(1, 4), nullptr);
}