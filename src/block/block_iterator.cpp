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

BlockIterator &BlockIterator::operator++()
{
    if (block && current_index < block->offsets.size())
    {
        ++current_index;
        cached_value = std::nullopt;
    }
    return *this;
}

bool BlockIterator::operator==(const BlockIterator &other) const
{
    if (block == nullptr && other.block == nullptr)
    {
        return true;
    }

    if (block == nullptr || other.block == nullptr)
    {
        return false;
    }

    auto cmp = block == other.block && current_index == other.current_index;
    return cmp;
}

bool BlockIterator::operator!=(const BlockIterator &other) const
{
    return !(*this == other);
}

BlockIterator::value_type BlockIterator::operator*() const
{
    if (!block || current_index >= block->size())
    {
        throw std::out_of_range("BlockIterator out of range");
    }

    // 使用缓存避免重复解析
    if (!cached_value.has_value())
    {
        update_current();
    }
    return *cached_value;
}

bool BlockIterator::is_end()
{
    return current_index == block->offsets.size();
}

void BlockIterator::update_current() const
{
    if (!cached_value && current_index < block->offsets.size())
    {
        size_t offset = block->get_offset_at(current_index);
        cached_value = std::make_pair(block->get_key_at(offset), block->get_value_at(offset));
    }
}