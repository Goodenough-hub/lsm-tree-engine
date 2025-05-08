#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include "../../include/block/block_iterator.h"
#include "../../include/iterator/iterator.h"

class SST;
class SstIterator;

// 返回的是第一个满足谓词的位置, 和最后一个满足谓词位置的下一个位置
// 左闭右开区间
// predicated 返回值:
// 0: 满足条件
// >0: 不满足谓词, 需要往右移动
// <0: 不满足谓词, 需要往左移动
std::optional<std::pair<SstIterator, SstIterator>> sst_iters_monotony_predicate(std::shared_ptr<SST> sst, uint64_t max_tranc_id, std::function<int(const std::string &)> predicate);

class SstIterator : public BaseIterator
{
    friend std::optional<std::pair<SstIterator, SstIterator>> sst_iters_monotony_predicate(std::shared_ptr<SST> sst, uint64_t max_tranc_id, std::function<int(const std::string &)> predicate);
    friend class SST;
    using value_type = std::pair<std::string, std::string>;
    using pointer = value_type *;

private:
    std::shared_ptr<SST> m_sst;
    size_t m_block_idx;
    std::shared_ptr<BlockIterator> m_block_iter;
    mutable std::optional<value_type> cached_value;
    uint64_t max_tranc_id_;

    void update_current() const;
    void seek(const std::string &key);

public:
    void set_block_idx(size_t idx);
    void set_block_it(std::shared_ptr<BlockIterator> it);
    SstIterator(std::shared_ptr<SST> sst, uint64_t max_tranc_id) : m_sst(std::move(sst)), m_block_idx(0), cached_value(std::nullopt) {}
    SstIterator(std::shared_ptr<SST> sst, const std::string &key, uint64_t max_tranc_id);

    virtual BaseIterator &operator++() override;
    SstIterator operator++(int) = delete; // 方便后续虚函数的实现

    virtual bool operator==(const BaseIterator &other) const override;
    virtual bool operator!=(const BaseIterator &other) const override;
    virtual value_type operator*() const;
    virtual IteratorType get_type() const override;
    virtual bool is_end() const override;
    virtual bool is_valid() const override;
    virtual uint64_t get_tranc_id() const override;

    pointer operator->() const;

    uint64_t get_sst_id() const;

    static std::pair<HeapIterator, HeapIterator>merge_sst_iterator(std::vector<SstIterator> iter_vec,
                                                                 uint64_t tranc_id);
};