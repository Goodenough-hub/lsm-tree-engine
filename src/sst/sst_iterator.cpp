#include "../../include/sst/sst_iterator.h"
#include "../../include/sst/sst.h"
#include <memory>

// 谓词查询
std::optional<std::pair<SstIterator, SstIterator>> sst_iters_monotony_predicate(std::shared_ptr<SST> sst, std::function<int(const std::string &)> predicate)
{
    // 初始化，分别用于存储最终的起始迭代器和结束迭代器。初始值为std::nullopt，表示尚未找到有效结果。
    std::optional<SstIterator> final_begin = std::nullopt;
    std::optional<SstIterator> final_end = std::nullopt;

    // 遍历SST中的所有数据块，索引从0到sst->num_blocks() - 1。
    for (int block_idx = 0; block_idx < sst->num_blocks(); block_idx++)
    {
        auto block = sst->read_block(block_idx); // 读取索引为block_idx的数据块，返回一个指向该数据块的对象。

        BlockMeta &meta_i = sst->meta_entries[block_idx]; // 获取当前数据块的元信息（BlockMeta对象），包括该块的第一个键（first_key）和最后一个键（last_key）。

        // 使用predicate函数对当前数据块的first_key和last_key进行评估。
        // 排除不满足条件的数据块，减少不必要的计算。
        if (predicate(meta_i.first_key) < 0 && predicate(meta_i.last_key) > 0)
        {
            continue;
        }

        // 对当前数据块执行谓词查询，返回一个std::optional对象，包含一对BlockIterator（起始迭代器和结束迭代器）。
        auto result_i = block->get_monotony_predicate(predicate);

        if (result_i.has_value())
        {
            auto [i_begin, i_end] = result_i.value();

            if (!final_begin.has_value())
            {
                // 如果final_begin尚未设置（即第一次找到满足条件的结果），创建一个新的SstIterator对象temp_it。
                // 置temp_it的块索引为block_idx，并将其块内迭代器设置为i_begin。
                // 将temp_it赋值给final_begin。

                auto temp_it = SstIterator(sst);
                temp_it.set_block_idx(block_idx);
                temp_it.set_block_it(i_begin);
                final_begin = temp_it;
            }

            // 设置结束迭代器
            auto temp_it = SstIterator(sst);
            temp_it.set_block_idx(block_idx);
            temp_it.set_block_it(i_end);
            if (!temp_it.is_valid())
            {
                temp_it.set_block_it(nullptr); // temp_it无效，则将其块内迭代器设置为nullptr。
            }
            final_end = temp_it; // 将temp_it赋值给final_end。
        }
    }

    if (!final_begin.has_value() || !final_end.has_value()) // 如果任一值为空，返回std::nullopt，表示查询失败。
    {
        return std::nullopt;
    }
    return std::make_pair(final_begin.value(), final_end.value());
}

SstIterator::SstIterator(std::shared_ptr<SST> sst, const std::string &key) : m_sst(std::move(sst)), cached_value(std::nullopt)
{
    if (m_sst)
    {
        seek(key);
    }
}

void SstIterator::set_block_idx(size_t idx)
{
    m_block_idx = idx;
}

void SstIterator::set_block_it(std::shared_ptr<BlockIterator> it)
{
    m_block_iter = it;
}

void SstIterator::seek(const std::string &key)
{
    if (!m_sst)
    {
        m_block_iter = nullptr;
        return;
    }

    try
    {
        m_block_idx = m_sst->find_block_idx(key);
        if (m_block_idx == -1 || m_block_idx >= m_sst->num_blocks())
        {
            // 把迭代器置为end或者无效的状态
            m_block_iter = nullptr;
            m_block_idx = m_sst->num_blocks();
            return;
        }
        auto block = m_sst->read_block(m_block_idx);
        if (!block)
        {
            m_block_iter = nullptr;
            return;
        }
        m_block_iter = std::make_shared<BlockIterator>(block, key);
        if (m_block_iter->is_end())
        {
            // block没法定位到key
            m_block_iter = nullptr;
            m_block_idx = m_sst->num_blocks();
            return;
        }
    }
    catch (const std::exception &e)
    {
        m_block_iter = nullptr;
        return;
    }
}
SstIterator &SstIterator::operator++()
{
    if (!m_block_iter)
    {
        throw std::runtime_error("Block iterator in Sstiterator is null");
    }

    // 1.先自增block
    ++(*m_block_iter);

    // 2.需要判断自增后是否end了
    if (m_block_iter->is_end())
    {
        // 2.1 如果是end了，则需要自增block
        m_block_idx++;
        if (m_block_idx < m_sst->num_blocks())
        {
            auto new_block = m_sst->read_block(m_block_idx);
            BlockIterator new_blk_it(new_block, 0);
            (*m_block_iter) = new_blk_it;
        }
        else
        {
            m_block_iter = nullptr;
        }
    }
    return *this;
}

bool SstIterator::operator==(const SstIterator &other) const
{
    if (m_sst != other.m_sst || m_block_idx != other.m_block_idx)
    {
        return false;
    }
    if (!m_block_iter && !other.m_block_iter)
    {
        return false;
    }

    return *m_block_iter == *other.m_block_iter;
}

bool SstIterator::operator!=(const SstIterator &other) const
{
    return !(*this == other);
}

SstIterator::value_type SstIterator::operator*() const
{
    if (!m_block_iter)
    {
        throw std::runtime_error("Block iterator in Sstiterator is null");
    }
    return *(*m_block_iter);
}

SstIterator::pointer SstIterator::operator->() const
{
    update_current();
    return &(*cached_value);
}

void SstIterator::update_current() const
{
    if (!cached_value && m_block_iter && !m_block_iter->is_end())
    {
        cached_value = *(*m_block_iter);
    }
}