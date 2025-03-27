#include "../../include/engine/two_merge_iterator.h"
#include <memory>

TwoMergeIterator::TwoMergeIterator() {}

TwoMergeIterator::TwoMergeIterator(std::shared_ptr<BaseIterator> a, std::shared_ptr<BaseIterator> b)
    : it_a(std::move(a)), it_b(move(b))
{
    skip_it_b();
    choose_a = choose_it_a();
}

// 是否选择迭代器it_a
bool TwoMergeIterator::choose_it_a()
{
    if (it_a->is_end()) // 如果a到达末尾，则返回false
    {
        return false;
    }
    if (it_b->is_end()) // 如果b到达末尾，则返回true
    {
        return true;
    }

    // 如果两者均未到达末尾，比较it_a和it_b当前元素的键值，返回键值较小的那个对应的布尔值
    // it_a 的键值小于 it_b 的键值时返回 true
    return (**it_a).first < (**it_b).first;
}

void TwoMergeIterator::skip_it_b()
{
    // it_b未到末尾但无效，it_a 的键值小于 it_b 的键值。
    if (!it_b->is_end() && !it_b->is_valid() && (**it_a).first < (**it_b).first)
    {
        ++(*it_b); // 递增it_b
    }
}

BaseIterator &TwoMergeIterator::operator++()
{
    if (choose_a)
    {
        ++(*it_a);
    }
    else
    {
        ++(*it_b);
    }
    skip_it_b(); // 跳过重复的key，选择优先级更高的it_a
    choose_a = choose_it_a();
    return *this;
}

bool TwoMergeIterator::operator==(const BaseIterator &other) const
{
    if (get_type() != other.get_type())
    {
        return false;
    }
    auto other2 = dynamic_cast<const TwoMergeIterator &>(other);

    if (is_end() && other2.is_end())
    {
        return true;
    }

    if (is_end() || other2.is_end())
    {
        return false;
    }

    return (*it_a) == (*other2.it_a) && (*it_b) == (*other2.it_b);
}

bool TwoMergeIterator::operator!=(const BaseIterator &other) const
{
    return !(*this == other);
}

BaseIterator::value_type TwoMergeIterator::operator*() const
{
    if (choose_a)
    {
        return **it_a;
    }
    else
    {
        return **it_b;
    }
}

IteratorType TwoMergeIterator::get_type() const
{
    return IteratorType::TwoMergeIterator;
}

bool TwoMergeIterator::is_end() const
{
    if (it_a == nullptr && it_b == nullptr)
    {
        return true;
    }
    if (it_a == nullptr)
    {
        return it_b->is_end();
    }
    if (it_b == nullptr)
    {
        return it_a->is_end();
    }
    return it_a->is_end() && it_b->is_end();
}

bool TwoMergeIterator::is_valid() const
{
    if (it_a == nullptr && it_b == nullptr)
    {
        return false;
    }
    if (it_a == nullptr)
    {
        return it_b->is_valid();
    }
    if (it_b == nullptr)
    {
        return it_a->is_valid();
    }
    return it_a->is_valid() || it_b->is_valid();
}

BaseIterator::pointer TwoMergeIterator::operator->() const
{
    update_current();
    return current.get();
}

// 根据choose_a的值选择从it_a或it_b迭代器中获取当前值，并将其存储到current指针中。
void TwoMergeIterator::update_current() const
{
    if (choose_a)
    {
        current = std::make_shared<value_type>(**it_a);
    }
    else
    {
        current = std::make_shared<value_type>(**it_b);
    }
}