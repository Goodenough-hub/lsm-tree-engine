#include "../../include/engine/engine.h"
#include "../../include/const.h"
#include "../../include/sst/concat_iterator.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vector>

std::optional<std::pair<TwoMergeIterator, TwoMergeIterator>> LSMEngine::iter_monotony_predicate(uint64_t tranc_id, std::function<int(const std::string &)> predicate)
{
    // 1.先从内存部分查询
    auto mem_result = memtable.iter_monotony_predicate(tranc_id, predicate); // 从内存表查询符合单调行的结果

    // 2.再从SST中查询
    std::vector<SearchItem> item_vec; // 存储从SST收集的查询结果

    // 后续实现了高层sst后需要修改这里的逻辑

    // 遍历所有的SST文件，sst_id越小表示文件越旧
    for (auto &[sst_idx, sst] : ssts) // 结构化绑定分解SST索引和对象
    {
        auto result = sst_iters_monotony_predicate(sst, tranc_id, predicate); // 在单个SST中查询
        if (!result.has_value())                                              // 没有符合条件的结果则跳过
        {
            continue;
        }
        auto [it_begin, it_end] = result.value(); // 解包迭代器范围
        for (; it_begin != it_end && it_begin.is_valid(); ++it_begin)
        {
            if (tranc_id != 0 && it_begin.get_tranc_id() > tranc_id)
            {
                continue;
            }
            // 将键值对存入向量，sst_idx取负保证新文件优先级更高
            // 越古老的sst的idx越小，我们需要让新的SST优先放在堆顶
            // 反转符号
            item_vec.emplace_back(it_begin->first, it_begin->second, -sst_idx, 0, tranc_id);
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
        auto start = TwoMergeIterator(mem_start_ptr, l0_iter_ptr, tranc_id);            // 合并内存和SST的迭代器
        auto end = TwoMergeIterator(tranc_id);                                          // 结束标记迭代器
        return std::make_optional(std::make_pair(start, end));                          // 返回迭代器对
    }
    else // 内存表无结果时的处理
    {
        auto start = TwoMergeIterator(std::make_shared<HeapIterator>(), l0_iter_ptr, tranc_id); // 空内存迭代器
        auto end = TwoMergeIterator(tranc_id);
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
            auto sst = SST::open(sst_id, FileObj::open(sst_path, false), block_cache);
            ssts[sst_id] = sst;

            level_sst_ids[0].push_back(sst_id);
        }
        std::sort(level_sst_ids[0].begin(), level_sst_ids[0].end());
        std::reverse(level_sst_ids[0].begin(), level_sst_ids[0].end());
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
void LSMEngine::put(const std::string &key, const std::string &value, uint64_t tranc_id)
{
    memtable.put(key, value, tranc_id);

    if (memtable.get_cur_size() >= LSM_TOTAL_MEM_SIZE_LIMIT)
    {
        // 如果memtable太大就需要刷盘
        flush();
    }
}

void LSMEngine::put_batch(const std::vector<std::pair<std::string, std::string>> &kvs, uint64_t tranc_id)
{
    memtable.put_batch(kvs, tranc_id);

    if (memtable.get_total_size() >= LSM_TOTAL_MEM_SIZE_LIMIT)
    {
        // 如果memtable太大就需要刷盘
        flush();
    }
}

std::optional<std::pair<std::string, uint64_t>> LSMEngine::get(const std::string &key, uint64_t tranc_id)
{
    // 1.先从memtable中查找
    SkipListIterator value = memtable.get(key, tranc_id);
    if (value.is_valid())
    {
        if (value.get_value().size() > 0)
        {
            return std::make_pair(value.get_value(), value.get_tranc_id());
        }
        else
        {
            // 删除标记
            return std::nullopt;
        }
    }
    // 2. l0_sst查询
    std::shared_lock<std::shared_mutex> rlock(ssts_mtx);
    return sst_get_(key, tranc_id);
}
std::optional<std::pair<std::string, uint64_t>>
LSMEngine::sst_get_(const std::string &key, uint64_t tranc_id)
{
    // 2. l0_sst查询
    std::shared_lock<std::shared_mutex> lock(ssts_mtx);
    for (auto &sst_id : level_sst_ids[0])
    {
        std::shared_ptr<SST> sst = ssts[sst_id];
        auto res = sst->get(key, tranc_id);
        if (res != sst->end(tranc_id))
        {
            if ((res->second.size() > 0))
            {
                return std::make_pair(res->second, res.get_tranc_id());
            }
            else
            {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

void LSMEngine::remove(const std::string &key, uint64_t tranc_id)
{
    memtable.remove(key, tranc_id);
    if (memtable.get_cur_size() >= LSM_TOTAL_MEM_SIZE_LIMIT)
    {
        // 如果memtable太大就需要刷盘
        flush();
    }
}

void LSMEngine::remove_batch(const std::vector<std::string> &keys, uint64_t tranc_id)
{
    memtable.remove_batch(keys, tranc_id);
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

    if (level_sst_ids.find(0) != level_sst_ids.end() &&
        level_sst_ids[0].size() > LSM_SST_LEVEL_RATIO) {
            // 说明已经存在了level0的sst
            full_compact(0);
        }

    // 1.创建一个新的sst_id
    size_t new_sst_id =
      level_sst_ids[0].empty() ? 0 : level_sst_ids[0].front() + 1;

    // 2.构建SST
    SSTBuilder builder(LSM_BLOCK_MEM_LIMIT, true);

    // 3.将Memtable中的最旧的一个table(skiplist)写入SST
    auto path = get_sst_path(new_sst_id);
    auto new_sst = memtable.flush_last(builder, path, new_sst_id, this->block_cache);

    // 4.更新内存索引
    ssts[new_sst_id] = new_sst;

    // 5.更新id
    level_sst_ids[0].push_front(new_sst_id);
}

void LSMEngine::full_compact(size_t src_level) {
    // 先判断 compact 是否需要递归进行
    if (level_sst_ids[src_level + 1].size() >= LSM_SST_LEVEL_RATIO) {
        full_compact(src_level + 1);
    }

    auto old_level_id_x = level_sst_ids[src_level];
    auto old_level_id_y = level_sst_ids[src_level];

    std::vector<std::shared_ptr<SST>> new_ssts;
    std::vector<size_t> lx_ids(old_level_id_x.begin(), old_level_id_x.end());
    std::vector<size_t> ly_ids(old_level_id_x.begin(), old_level_id_x.end());

    if (src_level == 0) {
        new_ssts = full_l0_l1_compact(lx_ids, ly_ids);
    } else {
        new_ssts = full_lx_ly_compact(lx_ids, ly_ids, src_level + 1);
    }

    for (auto &old_sst_id : old_level_id_x) {
        ssts[old_sst_id]->del_sst();
        ssts.erase(old_sst_id);
    }
    for (auto &old_sst_id : old_level_id_y) {
        ssts[old_sst_id]->del_sst();
        ssts.erase(old_sst_id);
    }
    level_sst_ids[src_level].clear();
    level_sst_ids[src_level + 1].clear();

    cur_max_level = std::max(cur_max_level, src_level + 1);

    for (auto &new_sst : new_ssts) {
        auto sst_id = new_sst->get_sst_id();
        level_sst_ids[src_level + 1].push_back(sst_id);
        ssts[sst_id] = new_sst;
    }

    std::sort(level_sst_ids[src_level + 1].begin(),
                level_sst_ids[src_level + 1].end());
}
  
std::vector<std::shared_ptr<SST>>
LSMEngine::full_l0_l1_compact(const std::vector<size_t> &l0_ids,
                            const std::vector<size_t> &l1_ids) {

    std::vector<SstIterator> l0_iters;
    std::vector<std::shared_ptr<SST>> l1_ssts;

    for (auto &sst_id : l0_ids) {
        auto sst = ssts[sst_id];
        l0_iters.push_back(sst->begin(0));
    }

    for (auto &sst_id : l1_ids) {
        auto sst = ssts[sst_id];
        l1_ssts.push_back(sst);
    }

    auto [l0_begin, l0_end] = SstIterator::merge_sst_iterator(l0_iters, 0);

    std::shared_ptr<HeapIterator> l0_begin_ptr = std::make_shared<HeapIterator>();
    *l0_begin_ptr = l0_begin;

    std::shared_ptr<ConcatIterator> old_l1_begin_ptr =
        std::make_shared<ConcatIterator>(l1_ssts, 0);

    TwoMergeIterator it_begin(l0_begin_ptr, old_l1_begin_ptr, 0);

    return gen_ssts_from_iter(it_begin, get_sst_size(1), 1);
}
  
std::vector<std::shared_ptr<SST>>
LSMEngine::full_lx_ly_compact(const std::vector<size_t> &lx_ids,
                            const std::vector<size_t> &ly_ids,
                            size_t y_level) {
    std::vector<std::shared_ptr<SST>> lx_ssts;
    std::vector<std::shared_ptr<SST>> ly_ssts;

    for (auto &sst_id : lx_ids) {
        auto sst = ssts[sst_id];
        lx_ssts.push_back(sst);
    }

    for (auto &sst_id : ly_ids) {
        auto sst = ssts[sst_id];
        ly_ssts.push_back(sst);
    }

    std::shared_ptr<ConcatIterator> old_lx_begin_ptr =
        std::make_shared<ConcatIterator>(lx_ssts, 0);

    std::shared_ptr<ConcatIterator> old_ly_begin_ptr =
        std::make_shared<ConcatIterator>(lx_ssts, 0);

    TwoMergeIterator it_begin(old_lx_begin_ptr, old_ly_begin_ptr, 0);

    return gen_ssts_from_iter(it_begin, get_sst_size(y_level), 1);
}
  
std::vector<std::shared_ptr<SST>>
LSMEngine::gen_ssts_from_iter(BaseIterator &iter, size_t target_sst_size,
                            size_t target_sst_level) {
    std::vector<std::shared_ptr<SST>> new_ssts;
    auto new_sst_builder = SSTBuilder(LSM_BLOCK_MEM_SIZE_LIMIT, true);

    while (iter.is_valid() && !iter.is_end()) {
        new_sst_builder.add((*iter).first, (*iter).second, iter.get_tranc_id());
        ++iter;

        if (new_sst_builder.estimated_size() >= target_sst_size) {
        auto sst_id = next_sst_id++;
        std::string sst_path = get_sst_path(sst_id);
        auto new_sst =
            new_sst_builder.build(target_sst_level, sst_path, this->block_cache);
        new_ssts.push_back(new_sst);
        new_sst_builder = SSTBuilder(LSM_BLOCK_MEM_SIZE_LIMIT, true);
        }
    }
    if (new_sst_builder.estimated_size() > 0) {
        auto sst_id = next_sst_id++;
        std::string sst_path = get_sst_path(sst_id);
        auto new_sst =
            new_sst_builder.build(target_sst_level, sst_path, this->block_cache);
        new_ssts.push_back(new_sst);
    }

    return new_ssts;
}
  
size_t LSMEngine::get_sst_size(const size_t &level) {
    size_t sst_size = LSM_PER_MEM_SIZE_LIMIT;
    for (int i = 1; i <= level; i++) {
        sst_size *= LSM_SST_LEVEL_RATIO;
    }
    return sst_size;
}

// ****************LSM***********************

LSM::LSM(std::string path) : engine_(std::make_shared<LSMEngine>(path)),
     tran_(std::make_shared<TranManager>(path, IsolationLevel::ReadCommitted))
{
    tran_->set_engine(engine_);
    std::map<uint64_t, std::vector<Record>> check_recover_res = tran_->check_recover();

    for(auto &[tranc_id, records] : check_recover_res)
    {
        for(auto &record : records)
        {
            if(record.op_type_ == Record::OperationType::Put)
            {
                engine_->put(record.key_, record.value_, tranc_id);
            }
            else if(record.op_type_ == Record::OperationType::Delete)
            {
                engine_->remove(record.key_, tranc_id);
            }
        }
    }
    tran_->init_new_wal();
}

std::shared_ptr<TranContext>
LSM::begin_transaction(const enum IsolationLevel &isolation_level)
{
    return tran_->new_tranc(isolation_level);
}