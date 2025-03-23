# lsm-tree-engine

## day1
实现跳表skiplist：
- put
- get
- remove

## day2
skiplist迭代器
对跳表封装，memtable
- 一个可读可写的skiplist
- 一个list，frozen_tables

## day3

memtable并发控制：两把锁
- 不加锁的函数
- 大批量数据处理的函数

block编码

## day4
block & sst编码
SSTbuilder用于构建SSTable

## day5

### 内存映射
将磁盘文件的数据映射到内存中，用户通过修改内存就能修改磁盘文件。

也可以用来进程间通信。

### 相关函数
```c++
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
/*
    - 功能：将文件或者设备的数据映射到内存中
    - 参数：
        - void *adde: NULL, 由内核指定
        - length：要映射的数据的长度，这个值不能为0.建议使用文件的长度
            获取文件的长度：stat leek
        - prot：对申请的内存映射区的操作权限
            - PROT_READ：可读
            - PROT_WRITE：可写
            - PROT_EXEC：可执行
            - PROT_NONE：不可读不可写不可执行
            要操作映射内存，必须要有读的权限
            PROT_READ 、 PROT_READ | PROT_WRITE
        - flags：映射文件的方式
            - MAP_SHARED：映射到内存中的数据，对映射到内存中的数据进行修改，映射到内存中的数据也会被修改.自动同步，进程间通信，必须要设置这个选项
            - MAP_PRIVATE：映射到内存中的数据，对映射到内存中的数据进行修改，映射到内存中的数据不会被修改.不同步，内存映射区的数据改变了，对原来的文件不会修改，会重新创建一个新的文件。 
        - fd：需要映射的那个文件的文件描述符
            - open 函数获取,open的是一个磁盘文件
            - 注意：文件的大小不能为0，open指定的权限不能与prot参数有冲突.必须要小于open的权限
            prot: Prot_READ               open:只读/读写
            prot: Prot_READ | Prot_WRITE  open:读写
        - offset：映射到内存中的偏移量，必须是文件系统系统的一个整数倍，一般使用0，表示不偏移
    - 返回值：成功返回映射的内存首地址，失败返回 (void *)-1  MAP_FAILED
*/

int munmap(void *addr, size_t length);
/*
    - 功能：取消内存映射
    - 参数：
        - void *addr：要释放的内存首地址
        - size_t length：释放内存的长度，和mmap函数中的length值相同
*/


int msync(void *addr, size_t length, int flags);
/*
    - 功能：同步内存映射区到文件。将内存映射区域（通过 mmap 创建）与磁盘文件同步，确保内存中的修改持久化到物理存储，或使缓存失效以重新加载磁盘数据。
    - 参数：
        - addr：内存映射区的首地址
        - length：内存映射区的长度
        - flags：操作选项。控制同步行为的标志位。
            - MS_ASYNC：异步写入（立即返回，不保证完成）
            - MS_SYNC：同步写入（阻塞直到完成）
            - MS_INVALIDATE：使缓存失效（后续访问重新读取磁盘）
*/
```

获取文件大小
```c++
int size = lseek(fd, 0, SEEK_END);

struct stat st;
fstat(fd_, &st) == -1
cout << st.st_size << endl;
```

> cpp中reserve与resize函数的异同
> - reserve：为容器预留空间，不会改变容器的大小，只是改变容器的容量，如果空间不足，会自动扩容，不会改变容器的大小。
> - resize：为容器重新分配空间，会改变容器的大小，如果空间不足，会自动扩容，会改变容器的大小。

--- 

# day18：谓词查询

## skiplist 谓词查询
```c++
// 返回的是第一个满足谓词的位置，和最后一个满足谓词位置的下一个位置
// 左开右闭区间
// predicate返回值：
// 0：满足条件
// >0：不满足谓词，需要往右移动
// <0：不满足谓词，需要往左移动

// 自定义 predicate 函数
// int custom_predicate(const std::string &key) {
//     if (key >= "apple" && key < "banana") {
//         return 0; // 满足条件
//     } else if (key < "apple") {
//         return -1; // 需要往右移动
//     } else {
//         return 1; // 需要往左移动
//     }
// }
std::optional<std::pair<SkipListIterator, SkipListIterator>> SkipList::iters_monotony_predicate(std::function<int(const std::string &)> predicate)
{
  auto current = head; // 从头节点开始遍历

  // 分别用于存储区间的起始和结束迭代器，初始值为空
  SkipListIterator begin_iter(nullptr);
  SkipListIterator end_iter(nullptr);

  // 从最高层开始查找，根据谓词函数 predicate 查找第一个满足条件的节点。
  bool find1 = false;
  for (int i = current_level - 1; i >= 0; i--)
  {
    while (!find1)
    {
      auto forward_i = current->forward[i]; // 获取当前层节点的 forward 指针
      if (forward_i == nullptr)             // 如果前向指针为空，说明已经达到了该层的末尾
      {
        break;
      }
      auto direction = predicate(forward_i->key); // 调用谓词函数判断当前节点是否满足条件。
      if (direction == 0)                         // 如果返回值为0，表示找到满足条件的节点。
      {
        find1 = true;        // 标记已找到
        current = forward_i; // 更新当前节点为找到的节点。
        break;               // 跳出循环
      }
      else if (direction < 0) // 表示需要向右移动
      {
        break; // 停止当前层的查找
      }
      else // 返回值大于0，表示需要继续向右移动。
      {
        current = forward_i; // 更新当前节点为前向节点
      }
    }
  }
  if (!find1) // 没找到满足条件的节点，返回空结果
  {
    return std::nullopt;
  }

  // 此时找到的节点是第一个满足要求的节点，
  // 最左侧满足条件的节点（即区间的起始节点）

  // 记录当前位置，用于后续查找区间
  auto current_2 = current;

  // 向右查找区间的结束节点。
  for (int i = current->backward.size() - 1; i >= 0; i--) // 从高到低遍历 forward 指针。
  {
    while (true)
    {
      // td::weak_ptr::lock()
      // 作用：lock() 方法尝试将一个 std::weak_ptr 转换为 std::shared_ptr。如果 std::weak_ptr 指向的对象仍然存在，
      // 则返回一个指向该对象的 std::shared_ptr；否则返回一个空的 std::shared_ptr。
      // current->backward[i].lock() 尝试将 std::weak_ptr 转换为 std::shared_ptr<SkipListNode>。
      // 如果转换后的 std::shared_ptr 为空（即 nullptr），说明该 std::weak_ptr 指向的对象已经被销毁。
      // 如果转换后的 std::shared_ptr 等于头节点 head，说明已经到达跳跃表的起始位置。
      if (current->backward[i].lock() == nullptr || current->backward[i].lock() == head)
      {
        // 没有实际存储键值对的前向节点了
        break;
      }
      auto direction = predicate(current->backward[i].lock()->key);
      if (direction == 0) // 找到满足条件的节点
      {
        current = current->backward[i].lock();
        continue;
      }
      else if (direction > 0)
      {
        // 需要更小的步长
        break;
      }
      else
      {
        throw std::runtime_error("iters_monotony_predicate error: invalid direction");
      }
    }
  }

  begin_iter = SkipListIterator(current); // 找左端点

  // 找右端点
  for (int i = current_2->forward.size() - 1; i >= 0; i--)
  {
    while (true)
    {
      if (current_2->forward[i] == nullptr)
      {
        break;
      }
      auto direction = predicate(current_2->forward[i]->key);
      if (direction == 0)
      {
        current_2 = current_2->forward[i];
        continue;
      }
      else if (direction < 0)
      {
        // 需要更小的步长
        break;
      }
      else
      {
        throw std::runtime_error("iters_monotony_predicate error: invalid direction");
      }
    }
  }
  end_iter = SkipListIterator(current_2);
  ++end_iter;

  return std::make_optional(std::make_pair(begin_iter, end_iter));
}
```

## block谓词查询
二分的两个模板
```c++
// 返回的是第一个满足谓词的位置，和最后一个满足谓词位置的下一个位置
// 左闭右开区间
// predicated 返回值：
// 0：满足条件
// >0：不满足谓词，需要向右移动
// <0：不满足谓词，需要向左移动
std::optional<std::pair<std::shared_ptr<BlockIterator>, std::shared_ptr<BlockIterator>>> Block::get_monotony_predicate(std::function<int(const std::string &)> predicate)
{
    // 如果offsets为空，则表示当前块中没有数据，直接返回
    if (offsets.empty())
    {
        return std::nullopt;
    }

    // 第一次二分查找到第一个满足谓词的位置
    int left = 0;
    int right = offsets.size() - 1;
    int first = -1;       // 真正的区间的起始位置
    int first_first = -1; // 第一找到的谓词位置

    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        size_t mid_offset = offsets[mid];

        auto mid_key = get_key_at(mid_offset);
        int direction = predicate(mid_key);

        if (direction < 0)
        {
            // 目标在mid左侧
            right = mid - 1;
        }
        else if (direction > 0)
        {
            // 在目标mid右侧
            left = mid + 1;
        }
        else
        {
            // 目标在mid位置
            first = mid;
            if (first_first == -1)
            {
                first_first = mid;
            }
            // 继续判断左边是否符合
            right = mid - 1;
        }
    }

    if (first == -1)
    {
        return std::nullopt;
    }

    // 继续找到最后一个满足谓词的位置
    left = first_first;
    right = offsets.size() - 1;
    int last = -1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        size_t mid_offset = offsets[mid];

        auto mid_key = get_key_at(mid_offset);
        int direction = predicate(mid_key);

        if (direction < 0)
        {
            // 目标在mid左侧
            right = mid - 1;
        }
        else if (direction > 0)
        {
            // 目标在mid右侧
            throw std::runtime_error("block is not sorted");
        }
        else
        {
            // 目标在mid位置
            last = mid;
            // 继续判断右边是否符合
            left = mid + 1;
        }
    }
    auto it_begin = std::make_shared<BlockIterator>(shared_from_this(), last);
    auto it_end = std::make_shared<BlockIterator>(shared_from_this(), last + 1);

    return std::make_pair(it_begin, it_end);
}
```

## memtable谓词查询
```c++
std::optional<std::pair<HeapIterator, HeapIterator>> Memtable::iter_monotony_predicate(std::function<int(const std::string &)> predicate)
{
    // 汇总每个skiplist谓词查询的结果

    // 加锁，并发读取
    std::shared_lock<std::shared_mutex> lock1(frozen_mtx);
    std::shared_lock<std::shared_mutex> lock2(cur_mtx);

    std::vector<SearchItem> item_vec; // 存储满足谓词条件的查询结果。每个结果是一个SearchItem对象，包含键、值以及表的索引。

    // 对活跃表的执行谓词查询
    auto cur_result = current_table->iters_monotony_predicate(predicate);
    if (cur_result.has_value())
    {
        auto [cur_begin, cur_end] = cur_result.value();
        for (auto iter = cur_begin; iter != cur_end; iter++) // 遍历迭代器范围
        {
            item_vec.emplace_back(SearchItem(iter.get_key(), iter.get_value(), 0));
        }
    }

    int table_idx = 1;
    for (auto ft = frozen_tables.begin(); ft != frozen_tables.end(); ft++) // 遍历frozen_tables中的每个冻结表
    {
        auto table = *ft;
        auto result = table->iters_monotony_predicate(predicate);
        if (result.has_value())
        {
            auto [begin, end] = result.value();
            for (auto iter = begin; iter != end; iter++)
            {
                item_vec.emplace_back(iter.get_key(), iter.get_value(), table_idx);
            }
        }
        table_idx++;
    }

    return std::make_pair(HeapIterator(item_vec), HeapIterator());
}
```

## sst谓词查询
函数的主要逻辑如下：
- 遍历SST中的所有数据块，对每个块的元信息和内部数据执行谓词查询。
- 根据查询结果，设置最终的起始迭代器和结束迭代器。
- 返回一个包含起始迭代器和结束迭代器的std::optional对象，供调用者使用。

```c++
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
```