#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <string>

enum class IteratorType
{
    SkipListIterator,
    BlockIterator,
    SSTIterator,
    HeapIterator,
    TwoMergeIterator,
    ConcatIterator,
};

class BaseIterator
{
public:
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;
    using reference = value_type &;

    virtual BaseIterator &operator++() = 0;
    virtual bool operator==(const BaseIterator &other) const = 0;
    virtual bool operator!=(const BaseIterator &other) const = 0;
    virtual value_type operator*() const = 0;
    virtual IteratorType get_type() const = 0;
    virtual bool is_end() const = 0;
    virtual bool is_valid() const = 0;
    virtual uint64_t get_tranc_id() const = 0;
};

struct SearchItem
{
    std::string key_;
    std::string value_;
    uint64_t tranc_id_;
    int idx_;
    int level_;

    SearchItem() = default;
    SearchItem(std::string k, std::string v, int i, int l, uint64_t tranc_id) : key_(k), value_(v), idx_(i), level_(l), tranc_id_(tranc_id) {}
};

bool operator<(const SearchItem &a, const SearchItem &b);
bool operator>(const SearchItem &a, const SearchItem &b);
bool operator==(const SearchItem &a, const SearchItem &b);

class HeapIterator : public BaseIterator
{
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;
    using reference = value_type &;

    friend class SSTIterator;

public:
    HeapIterator() = default;
    HeapIterator(std::vector<SearchItem> item_vec, uint64_t tranc_id);
    virtual value_type operator*() const override;
    virtual bool operator==(const BaseIterator &rhs) const override;
    virtual bool operator!=(const BaseIterator &rhs) const override;
    virtual BaseIterator &operator++() override;
    virtual BaseIterator operator++(int) = delete;
    virtual pointer operator->() const;
    virtual IteratorType get_type() const override;
    virtual uint64_t get_tranc_id() const override;
    virtual bool is_end() const override;
    virtual bool is_valid() const override;

private:
    bool top_value_legal() const;
    void update_current() const;
    void skip_by_tranc_id();

private:
    // 小根堆
    std::priority_queue<SearchItem, std::vector<SearchItem>, std::greater<SearchItem>> items;
    mutable std::shared_ptr<value_type> current;
    uint64_t max_tranc_id_;

    // 添加转换函数
    static value_type to_pair(const SearchItem &item)
    {
        return std::make_pair(item.key_, item.value_);
    }
};