#include "../../include/block/block_iterator.h"
#include "../../include/block/block.h"
#include <optional>

BlockIterator::BlockIterator(std::shared_ptr<Block> b, size_t index, uint64_t tranc_id)
    : block(std::move(b)), current_index(index), cached_value(std::nullopt), max_tranc_id_(tranc_id) {}

BlockIterator::BlockIterator(std::shared_ptr<Block> b, const std::string &key, uint64_t tranc_id)
    : block(std::move(b)), cached_value(std::nullopt), max_tranc_id_(tranc_id)
{
    auto key_idx_ops = block->get_idx_binary(key, tranc_id);
    if (key_idx_ops.has_value())
    {
        current_index = key_idx_ops.value();
    }
    else
    {
        current_index = block->offsets.size();
    }
}

BaseIterator &BlockIterator::operator++()
{
    if (block && current_index < block->offsets.size())
    {
        ++current_index;
        cached_value = std::nullopt;
    }
    return *this;
}

bool BlockIterator::operator==(const BaseIterator &other) const
{
    if (other.get_type() != IteratorType::BlockIterator)
    {
        return false;
    }
    auto other2 = dynamic_cast<const BlockIterator &>(other);
    if (block == nullptr && other2.block == nullptr)
    {
        return true;
    }

    if (block == nullptr || other2.block == nullptr)
    {
        return false;
    }

    auto cmp = block == other2.block && current_index == other2.current_index;
    return cmp;
}

bool BlockIterator::operator!=(const BaseIterator &other) const
{
    return !(*this == other);
}

BlockIterator::pointer BlockIterator::operator->() const
{
    update_current();        // 更新当前状态。
    return &(*cached_value); // 返回缓存值的指针。
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

bool BlockIterator::is_end() const
{
    return current_index == block->offsets.size();
}

bool BlockIterator::is_valid() const
{
    return !(current_index == block->offsets.size());
}

IteratorType BlockIterator::get_type() const
{
    return IteratorType::BlockIterator;
}

uint64_t BlockIterator::get_tranc_id() const
{
    return max_tranc_id_;
}
void BlockIterator::update_current() const
{
    if (!cached_value && current_index < block->offsets.size())
    {
        size_t offset = block->get_offset_at(current_index);
        cached_value = std::make_pair(block->get_key_at(offset), block->get_value_at(offset));
    }
}