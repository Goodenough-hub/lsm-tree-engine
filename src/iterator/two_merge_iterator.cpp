#include "../../include/engine/two_merge_iterator.h"

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