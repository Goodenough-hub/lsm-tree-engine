#include "../../include/sst/concat_iterator.h"
#include "../../include/sst/sst.h"

ConcatIterator::ConcatIterator(std::vector<std::shared_ptr<SST>> ssts,
                               uint64_t max_tranc_id)
    : ssts_(std::move(ssts)), cur_iter(nullptr, 0), cur_idx(0),
      max_tranc_id_(max_tranc_id) {
  if (!ssts_.empty()) {
    cur_iter = ssts_[cur_idx]->begin(max_tranc_id_);
  }
}

BaseIterator &ConcatIterator::operator++() {
  ++cur_iter;
  if (cur_iter.is_end() || !cur_iter.is_valid()) {
    ++cur_idx;
    if (cur_idx < ssts_.size()) {
      cur_iter = ssts_[cur_idx]->begin(max_tranc_id_);
    } else {
      cur_iter = SstIterator(nullptr, max_tranc_id_);
    }
  }
  return *this;
}

bool ConcatIterator::operator==(const BaseIterator &other) const {
  if (other.get_type() != IteratorType::ConcactIterator) {
    return false;
  }
  if (auto other_iter = dynamic_cast<const ConcatIterator *>(&other)) {
    return cur_iter == other_iter->cur_iter && cur_idx == other_iter->cur_idx;
  }
  return false;
}

bool ConcatIterator::operator!=(const BaseIterator &other) const {
  return !(*this == other);
}

ConcatIterator::value_type ConcatIterator::operator*() const {
  return *cur_iter;
}

IteratorType ConcatIterator::get_type() const {
  return IteratorType::ConcactIterator;
}

bool ConcatIterator::is_end() const {
  return cur_iter.is_end() && cur_idx >= ssts_.size();
}
bool ConcatIterator::is_valid() const {
  return cur_iter.is_valid() && cur_idx < ssts_.size();
}