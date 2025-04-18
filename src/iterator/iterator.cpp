#include "../../include/iterator/iterator.h"

bool operator<(const SearchItem &a, const SearchItem &b)
{
    if (a.key_ != b.key_)
    {
        return a.key_ < b.key_;
    }
    if (a.tranc_id_ > b.tranc_id_)
    {
        return true;
    }
    if (a.level_ < b.level_)
    {
        return true;
    }
    return a.idx_ < b.idx_;
}

bool operator>(const SearchItem &a, const SearchItem &b)
{
    if (a.key_ != b.key_)
    {
        return a.key_ > b.key_;
    }
    if (a.tranc_id_ < b.tranc_id_)
    {
        return true;
    }
    if (a.level_ > b.level_)
    {
        return true;
    }
    return a.idx_ > b.idx_;
}

bool operator==(const SearchItem &a, const SearchItem &b)
{
    return a.key_ == b.key_ && a.idx_ == b.idx_;
}

HeapIterator::HeapIterator(std::vector<SearchItem> item_vec, uint64_t max_tranc_id) : max_tranc_id_(max_tranc_id)
{
    for (auto &item : item_vec)
    {
        items.push(item);
    }

    while (!top_value_legal())
    {
        // 1. 跳过事务id不可见的部分
        skip_by_tranc_id();

        // 2. 跳过标记为删除的元素
        while (!items.empty() && items.top().value_.empty())
        {
            auto del_key = items.top().key_;
            while (!items.empty() && items.top().key_ == del_key)
            {
                items.pop();
            }
        }
    }
}

bool HeapIterator::top_value_legal() const
{
    if (items.empty())
    {
        return true;
    }
    if (max_tranc_id_ == 0)
    {
        // 没有开启事务
        return items.top().value_.size() > 0;
    }

    if (items.top().tranc_id_ <= max_tranc_id_)
    {
        return items.top().value_.size() > 0;
    }
    else
    {
        return false;
    }
}

void HeapIterator::skip_by_tranc_id()
{
    if (max_tranc_id_ == 0)
        while (!items.empty() && items.top().tranc_id_ > max_tranc_id_)
        {
            items.pop();
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

    return items.top().key_ == other2.items.top().key_ && items.top().value_ == other2.items.top().value_;
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
    while (!items.empty() && items.top().key_ == old_item.key_)
    {
        items.pop();
    }

    while (!top_value_legal())
    {
        // 1. 跳过事务id不可见的部分
        skip_by_tranc_id();

        // 2. 跳过标记为删除的元素
        while (!items.empty() && items.top().value_.empty())
        {
            auto del_key = items.top().key_;
            while (!items.empty() && items.top().key_ == del_key)
            {
                items.pop();
            }
        }
    }

    return *this;
}

BaseIterator::value_type HeapIterator::operator*() const
{
    // 定义了HeapIterator类中解引用操作符*的行为，返回堆顶元素的键值对。
    return std::make_pair(items.top().key_, items.top().value_);
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
        current = std::make_shared<value_type>(items.top().key_, items.top().value_);
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