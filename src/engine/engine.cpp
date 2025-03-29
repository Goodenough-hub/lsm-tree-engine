#include "../../include/engine/engine.h"
#include "../../include/const.h"
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vector>

std::optional<std::pair<TwoMergeIterator, TwoMergeIterator>> LSMEngine::iter_monotony_predicate(std::function<int(const std::string &)> predicate)
{
    // 1.先从内存部分查询
    auto mem_result = memtable.iter_monotony_predicate(predicate); // 从内存表查询符合单调行的结果

    // 2.再从SST中查询
    std::vector<SearchItem> item_vec; // 存储从SST收集的查询结果

    // 后续实现了高层sst后需要修改这里的逻辑

    // 遍历所有的SST文件，sst_id越小表示文件越旧
    for (auto &[sst_idx, sst] : ssts) // 结构化绑定分解SST索引和对象
    {
        auto result = sst_iters_monotony_predicate(sst, predicate); // 在单个SST中查询
        if (!result.has_value())                                    // 没有符合条件的结果则跳过
        {
            continue;
        }
        auto [it_begin, it_end] = result.value(); // 解包迭代器范围
        for (; it_begin != it_end && it_begin.is_valid(); ++it_begin)
        {
            // 将键值对存入向量，sst_idx取负保证新文件优先级更高
            // 越古老的sst的idx越小，我们需要让新的SST优先放在堆顶
            // 反转符号
            item_vec.emplace_back(it_begin->first, it_begin->second, -sst_idx);
        }
    }

    // 3.合并结果
    std::shared_ptr<HeapIterator> l0_iter_ptr = make_shared<HeapIterator>(item_vec);

    // 4.构造返回结果
    if (!mem_result.has_value()) // 内存表有查询结果时的处理
    {
        auto [mem_start, mem_end] = mem_result.value();                                 // 解包内存表迭代器
        std::shared_ptr<HeapIterator> mem_start_ptr = std::make_shared<HeapIterator>(); // 创建智能指针
        *mem_start_ptr = mem_start;                                                     // 复制迭代器状态 ==> 解引用
        auto start = TwoMergeIterator(mem_start_ptr, l0_iter_ptr);                      // 合并内存和SST的迭代器
        auto end = TwoMergeIterator();                                                  // 结束标记迭代器
        return std::make_optional(std::make_pair(start, end));                          // 返回迭代器对
    }
    else // 内存表无结果时的处理
    {
        auto start = TwoMergeIterator(std::make_shared<HeapIterator>(), l0_iter_ptr); // 空内存迭代器
        auto end = TwoMergeIterator();
        return std::make_optional(std::make_pair(start, end));
    }
}
LSMEngine::LSMEngine(const std::string path) : data_dir(path)
{
    block_cache = std::make_shared<BlockCache>(LSM_BLOCK_CACHE_CAPACITY, LSM_BLOCK_CACHE_K);

    // 判断数据库文件
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directories(path);
    }
    else
    {
        // 检查sst文件并加载
        for (auto &entry : std::filesystem::directory_iterator(path))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }
            std::string filename = entry.path().filename().string();
            if (filename.substr(0, 4) != "sst_")
            {
                continue;
            }
            // 提取sst id
            // sst_{id} id 占4位
            std::string id_str = filename.substr(4, filename.length() - 4);
            if (id_str.empty())
            {
                continue;
            }
            size_t sst_id = std::stoi(id_str);

            // 加载sst文件
            std::unique_lock<std::shared_mutex> lock(ssts_mtx);
            std::string sst_path = get_sst_path(sst_id);
            auto sst = SST::open(sst_id, FileObj::open(sst_path), block_cache);
            ssts[sst_id] = sst;

            l0_sst_ids.push_back(sst_id);
        }
        l0_sst_ids.sort();
        l0_sst_ids.reverse();
    }
}

LSMEngine::~LSMEngine()
{
    while (memtable.get_total_size() > 0)
    {
        // 刷盘
        flush();
    }
}
void LSMEngine::put(const std::string &key, const std::string &value)
{
    memtable.put(key, value);

    if (memtable.get_cur_size() >= LSM_TOTAL_MEM_SIZE_LIMIT)
    {
        // 如果memtable太大就需要刷盘
        flush();
    }
}

void LSMEngine::put_batch(const std::vector<std::pair<std::string, std::string>> &kvs)
{
    memtable.put_batch(kvs);

    if (memtable.get_total_size() >= LSM_TOTAL_MEM_SIZE_LIMIT)
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
        if (value->size() > 0)
        {
            return value;
        }
        else
        {
            // 删除标记
            return std::nullopt;
        }
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

void LSMEngine::remove(const std::string &key)
{
    memtable.remove(key);
    if (memtable.get_cur_size() >= LSM_TOTAL_MEM_SIZE_LIMIT)
    {
        // 如果memtable太大就需要刷盘
        flush();
    }
}

void LSMEngine::remove_batch(const std::vector<std::string> &keys)
{
    memtable.remove_batch(keys);
    if (memtable.get_cur_size() >= LSM_TOTAL_MEM_SIZE_LIMIT)
    {
        // 如果memtable太大就需要刷盘
        flush();
    }
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
    SSTBuilder builder(LSM_BLOCK_MEM_LIMIT, true);

    // 3.将Memtable中的最旧的一个table(skiplist)写入SST
    auto path = get_sst_path(new_sst_id);
    auto new_sst = memtable.flush_last(builder, path, new_sst_id, this->block_cache);

    // 4.更新内存索引
    ssts[new_sst_id] = new_sst;

    // 5.更新id
    l0_sst_ids.push_front(new_sst_id);
}
