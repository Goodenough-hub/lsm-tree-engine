#include "../../include/sst/sst_iterator.h"
#include "../../include/sst/sst.h"

SstIterator::SstIterator(std::shared_ptr<SST> sst, const std::string &key) : m_sst(std::move(sst)), cached_value(std::nullopt)
{
    if (m_sst)
    {
        seek(key);
    }
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