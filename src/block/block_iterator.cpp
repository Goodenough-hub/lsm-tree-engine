#include "../../include/block/block_iterator.h"
#include "../../include/block/block.h"
#include <optional>

BlockIterator::BlockIterator(std::shared_ptr<Block> b, size_t index) : block(std::move(b)), current_index(index), cached_value(std::nullopt) {}

BlockIterator::BlockIterator(std::shared_ptr<Block> b, const std::string &key) : block(std::move(b)), cached_value(std::nullopt)
{
    auto key_idx_ops = block->get_idx_binary(key);
    if (key_idx_ops.has_value())
    {
        current_index = key_idx_ops.value();
    }
    else
    {
        current_index = block->offsets.size();
    }
}