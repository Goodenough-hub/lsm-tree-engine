#pragma once

#include "../iterator/iterator.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>

class Block;
class BlockIterator : public BaseIterator
{
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;

private:
    std::shared_ptr<Block> block;
    size_t current_index;
    mutable std::optional<value_type> cached_value;
    uint64_t max_tranc_id_;

public:
    BlockIterator(std::shared_ptr<Block> b, size_t index, uint64_t tranc_id);
    BlockIterator(std::shared_ptr<Block> b, const std::string &key, uint64_t tranc_id);

    BlockIterator::pointer operator->() const;

    virtual BaseIterator &operator++() override;
    virtual bool operator==(const BaseIterator &other) const override;
    virtual bool operator!=(const BaseIterator &other) const override;
    virtual value_type operator*() const override;
    virtual IteratorType get_type() const override;
    virtual bool is_end() const override;
    virtual bool is_valid() const override;
    virtual uint64_t get_tranc_id() const override;

    void update_current() const;
};