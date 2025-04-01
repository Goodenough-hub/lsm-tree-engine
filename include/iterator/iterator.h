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
};

struct SearchItem
{
    std::string key;
    std::string value;
    int idx;

    SearchItem() = default;
    SearchItem(std::string k, std::string v, int i) : key(k), value(v), idx(i) {}
};

bool operator<(const SearchItem &a, const SearchItem &b);
bool operator>(const SearchItem &a, const SearchItem &b);
bool operator==(const SearchItem &a, const SearchItem &b);

class HeapIterator : public BaseIterator
{
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;
    using reference = value_type &;

public:
    HeapIterator() = default;
    HeapIterator(std::vector<SearchItem> item_vec);
    virtual value_type operator*() const override;
    virtual bool operator==(const BaseIterator &rhs) const override;
    virtual bool operator!=(const BaseIterator &rhs) const override;
    virtual BaseIterator &operator++() override;
    virtual BaseIterator operator++(int) = delete;
    virtual pointer operator->() const;
    virtual IteratorType get_type() const override;
    virtual bool is_end() const override;
    virtual bool is_valid() const override;

private:
    // 小根堆
    std::priority_queue<SearchItem, std::vector<SearchItem>, std::greater<SearchItem>> items;
    mutable std::shared_ptr<value_type> current;
    void update_current() const;

    // 添加转换函数
    static value_type to_pair(const SearchItem &item)
    {
        return std::make_pair(item.key, item.value);
    }
};