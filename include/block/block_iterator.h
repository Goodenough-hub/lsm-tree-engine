#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

class Block;
class BlockIterator
{
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;

private:
    std::shared_ptr<Block> block;
    size_t current_index;
    mutable std::optional<value_type> current_value;

public:
    BlockIterator(std::shared_ptr<Block> b, size_t index);

    BlockIterator &operator++();
    BlockIterator operator++(int) = delete;

    bool operator==(const BlockIterator &other) const;
    bool operator!=(const BlockIterator &other) const;
    value_type operator*() const;
    bool is_end();
}