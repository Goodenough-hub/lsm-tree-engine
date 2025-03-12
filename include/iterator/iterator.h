#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <string>

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

class HeapIterator
{
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;
    using reference = value_type &;

public:
    HeapIterator() = default;
    HeapIterator(std::vector<SearchItem> item_vec);
    virtual bool operator==(const HeapIterator &rhs) const;
    virtual bool operator!=(const HeapIterator &rhs) const;
    HeapIterator &operator++();
    HeapIterator operator++(int) = delete;
    virtual pointer operator->() const;
    virtual bool is_end() const;

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