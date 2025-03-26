#pragma once

#include "../iterator/iterator.h"
#include <memory>

class TwoMergeIterator : public BaseIterator
{
private:
    std::shared_ptr<BaseIterator> it_a; // example: memtable iterator (Heapiterator)
    std::shared_ptr<BaseIterator> it_b; // example: sst iterator (SstIterator)

    bool choose_a = false;
    mutable std::shared_ptr<value_type> current;

    void update_current();

public:
    TwoMergeIterator();
    TwoMergeIterator(std::shared_ptr<BaseIterator> a, std::shared_ptr<BaseIterator> b);

    bool choose_it_a(); // 是否选择迭代器it_a
    void skip_it_b();

    virtual BaseIterator &operator++() override;
    virtual bool operator==(const BaseIterator &other) const override;
    virtual bool operator!=(const BaseIterator &other) const override;
    virtual value_type operator*() const override;
    virtual IteratorType get_type() const override;
    virtual bool is_end() const override;
    virtual bool is_valid() const override;
};