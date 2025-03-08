#include "../../include/engine/engin.h"
#include "../../include/const.h"
#include <sstream>
#include <iomanip>

void LSMEngine::put(const std::string &key, const std::string &value)
{
    memtable.put(key, value);

    if (memtable.get_cur_size() > LSM_TOTAL_MEM_SIZE_LIMIT)
    {
        // 如果memtable太大就需要刷盘
        flush();
    }
}

std::optional<std::string> LSMEngine::get(const std::string &key)
{
    // 1.先从memtable中查找
    auto value = memtable.get(key);
    if (value.has_value())
    {
        return value;
    }
    else
    {
        // 删除标记
        return std::nullopt;
    }

    // 2. l0_sst查询
    std::shared_lock<std::shared_mutex> lock(ssts_mtx);
    for (auto &sst_id : l0_sst_ids)
    {
        std::shared_ptr<SST> sst = ssts[sst_id];
        auto res = sst->get(key);
        if (res != sst->end())
        {
            if ((res->second.size() > 0))
            {
                return res->second;
            }
            else
            {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

std::string LSMEngine::get_sst_path(size_t sst_id)
{
    // sst的文件格式：data_dir/sst_<sst_id>
    std::stringstream ss;
    ss << data_dir << "/sst_" << std::setfill('0') << std::setw(4) << sst_id;
    return ss.str();
}

void LSMEngine::flush()
{
    if (memtable.get_total_size() == 0)
    {
        return;
    }

    // 1.创建一个新的sst_id
    size_t new_sst_id = l0_sst_ids.empty() ? 0 : l0_sst_ids.front() + 1;

    // 2.构建SST
    SSTBuilder builder(LSM_BLOCK_MEM_LIMIT);

    // 3.将Memtable中的最旧的一个table(skiplist)写入SST
    auto path = get_sst_path(new_sst_id);
    auto new_sst = memtable.flush_last(builder, path, new_sst_id);

    // 4.更新内存索引
    ssts[new_sst_id] = new_sst;

    // 5.更新id
    l0_sst_ids.push_front(new_sst_id);
}
