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

public:
    BlockIterator(std::shared_ptr<Block> b, size_t index);
    BlockIterator(std::shared_ptr<Block> b, const std::string &key);

    BlockIterator::pointer operator->() const;

    virtual BaseIterator &operator++() override;
    virtual bool operator==(const BaseIterator &other) const override;
    virtual bool operator!=(const BaseIterator &other) const override;
    virtual value_type operator*() const override;
    virtual IteratorType get_type() const override;
    virtual bool is_end() const override;
    virtual bool is_valid() const override;

    void update_current() const;
};