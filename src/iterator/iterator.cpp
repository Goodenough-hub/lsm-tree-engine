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

bool HeapIterator::operator==(const BaseIterator &other) const
{
    if (get_type() != other.get_type())
    {
        return false;
    }
    auto other2 = dynamic_cast<const HeapIterator &>(other);
    if (items.empty() && other2.items.empty())
    {
        return true;
    }

    if (items.empty() || other2.items.empty())
    {
        return false;
    }

    return items.top().key == other2.items.top().key && items.top().value == other2.items.top().value;
}

bool HeapIterator::operator!=(const BaseIterator &rhs) const
{
    return !(*this == rhs);
}

BaseIterator &HeapIterator::operator++() // 前置++
{
    // 检查堆是否为空，若为空直接返回当前迭代器
    if (items.empty())
    {
        return *this;
    }

    // 弹出堆顶元素
    auto old_item = items.top();
    items.pop();

    // 删除与旧元素（堆顶元素）key相同的元素
    while (!items.empty() && items.top().key == old_item.key)
    {
        items.pop();
    }

    // 删除堆中所有value为空的元素及其key相同的元素
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

BaseIterator::value_type HeapIterator::operator*() const
{
    // 定义了HeapIterator类中解引用操作符*的行为，返回堆顶元素的键值对。
    return std::make_pair(items.top().key, items.top().value);
}

BaseIterator::pointer HeapIterator::operator->() const
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

bool HeapIterator::is_valid() const
{
    return !items.empty();
}

IteratorType HeapIterator::get_type() const
{
    return IteratorType::HeapIterator;
}