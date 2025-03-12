#include "../../include/iterator/iterator.h"

bool operator<(const SearchItem &a, const SearchItem &b)
{
    if (a.key != b.key)
    {
        return a.key < b.key;
    }
    return a.idx < b.idx;
}

bool operator>(const SearchItem &a, const SearchItem &b)
{
    if (a.key != b.key)
    {
        return a.key > b.key;
    }
    return a.idx > b.idx;
}

bool operator==(const SearchItem &a, const SearchItem &b)
{
    return a.key == b.key && a.idx == b.idx;
}

HeapIterator::HeapIterator(std::vector<SearchItem> item_vec)
{
    for (auto &item : item_vec)
    {
        items.push(item);
    }

    while (!items.empty() && items.top().value.empty()) // 删除堆中value为空的元素
    {
        auto del_key = items.top().key;
        while (!items.empty() && items.top().key == del_key)
        {
            items.pop();
        }
    }
}

bool HeapIterator::operator==(const HeapIterator &rhs) const
{
    if (items.empty() && rhs.items.empty())
    {
        return true;
    }

    if (items.empty() || rhs.items.empty())
    {
        return false;
    }

    return items.top().key == rhs.items.top().key && items.top().value == rhs.items.top().value;
}

bool HeapIterator::operator!=(const HeapIterator &rhs) const
{
    return !(*this == rhs);
}

HeapIterator &HeapIterator::operator++()
{
    if (items.empty())
    {
        return *this;
    }

    auto old_item = items.top();
    items.pop();

    // 删除与旧元素key相同的元素
    while (!items.empty() && items.top().key == old_item.key)
    {
        items.pop();
    }

    while (!items.empty() && items.top().value.empty())
    {
        auto del_key = items.top().key;
        while (!items.empty() && items.top().key == del_key)
        {
            items.pop();
        }
    }

    return *this;
}

HeapIterator::pointer HeapIterator::operator->() const
{
    update_current();
    return current.get(); // 获取原始指针
}

void HeapIterator::update_current() const
{
    if (!items.empty())
    {
        current = std::make_shared<value_type>(to_pair(items.top()));
    }
    else
    {
        current.reset(); // 重新分配shared_ptr所拥有的指针
    }
}

bool HeapIterator::is_end() const
{
    return items.empty();
}