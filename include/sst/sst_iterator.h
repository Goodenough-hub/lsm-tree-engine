#pragma once

#include <memory>
#include <optional>
#include "../../include/block/block_iterator.h"

class SST;

class SstIterator
{
    friend class SST;
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;

private:
    std::shared_ptr<SST> m_sst;
    size_t m_block_idx;
    std::shared_ptr<BlockIterator> m_block_iter;
    mutable std::optional<value_type> cached_value;

    void update_current() const;

public:
    SstIterator(std::shared_ptr<SST> sst) : m_sst(std::move(sst)), m_block_idx(0), cached_value(std::nullopt) {}
    SstIterator(std::shared_ptr<SST> sst, const std::string &key) : m_sst(std::move(sst)), m_block_idx(0), cached_value(std::nullopt) {}
    SstIterator &operator++();
    SstIterator operator++(int) = delete; // 方便后续虚函数的实现

    bool operator==(const SstIterator &other) const;
    bool operator!=(const SstIterator &other) const;
    value_type operator*() const;
    pointer operator->() const;
    bool is_valid();
};