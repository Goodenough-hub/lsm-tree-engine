#pragma once

#include "sst_iterator.h"
#include <cstddef>
#include <memory>
#include <vector>

class SST;

class ConcatIterator : public BaseIterator {
private:
    SstIterator cur_iter;
    size_t cur_idx;
    std::vector<std::shared_ptr<SST>> ssts_;
    uint64_t max_tranc_id_;

public:
    ConcatIterator(std::vector<std::shared_ptr<SST>> ssts, uint64_t max_tranc_id);

    virtual BaseIterator &operator++() override;
    virtual bool operator==(const BaseIterator &other) const override;
    virtual bool operator!=(const BaseIterator &other) const override;
    virtual value_type operator*() const override;
    virtual IteratorType get_type() const override;
    virtual bool is_end() const override;
    virtual bool is_valid() const override;
    virtual uint64_t get_tranc_id() const override;
};