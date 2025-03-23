#include "../../include/skiplist/skiplist.h"
#include <cstddef>
#include <stdexcept>
#include <tuple>

/*********************** SkipList *******************/

// 构造函数
SkipList::SkipList(int max_level)
{
  this->max_level = max_level;
  this->head = std::make_shared<SkipListNode>("", "", max_level); // 创建头节点
  this->current_level = 1;

  // 随机数
  this->dis_01 = std::uniform_int_distribution<>(0, 1);
  this->dis_level = std::uniform_int_distribution<>(0, (1 << max_level) - 1);
  this->gen = std::mt19937(this->rd());
}

int SkipList::random_level()
{
  int level = 1;
  // 通过抛硬币的方式生成随机层数
  // 每一次抛硬币都有50%的概率增加一层
  while (dis_01(gen) && level < max_level)
  { // 执行次数：[0, max_level - 1]
    level++;
  }
  return level; // [1, max_level]
}

void SkipList::put(const std::string &key, const std::string &value)
{
  //   if (value.empty()) {
  //     throw std::runtime_error("value cannot be empty"); // 值为空，抛出异常
  //   }

  //   标记当前的节点
  //   初始化当前节点为头节点 head，用于遍历跳表
  auto current = head;

  // 创建一个大小为 max_level 的向量 update，用于记录每一层的前驱节点。
  std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);

  // 从最高层开始向下遍历，找到每一层中小于给定 key 的最大节点，并将其记录到 update 中。
  for (int i = current_level - 1; i >= 0; i--)
  {
    while (current->forward[i] && current->forward[i]->key < key)
    {
      current = current->forward[i];
    }
    update[i] = current;
  }

  // 移动到最底层
  current = current->forward[0];

  // 判断 key 是否存在
  if (current && current->key == key)
  {
    size_bytes += value.size() - current->value.size();
    current->value = value;
    return;
  }

  //   value不存在则需要创建
  int new_level = std::max(random_level(), current_level);
  for (int i = current_level; i < new_level; ++i)
  {
    update[i] = head;
  }

  auto new_node = std::make_shared<SkipListNode>(key, value, new_level);
  size_bytes += new_node->key.size() + new_node->value.size();

  // 三种状态的update
  // 第0层一定更新
  // 如果创建了新的层级，所有层级都更新
  // 每次50%的概率更新，某层不更新则中断更新
  int radn_bits = dis_level(gen);
  for (int i = 0; i < new_level; ++i)
  {
    bool is_update = false;
    if (i == 0 || (new_level > current_level) || (radn_bits & (1 << i)))
    {
      is_update = true; // 需要更新
    }
    if (is_update)
    {
      // 更新forward与backward
      new_node->forward[i] = update[i]->forward[i];
      if (new_node->forward[i])
      {
        new_node->forward[i]->set_backward(i, new_node);
      }

      update[i]->forward[i] = new_node;
      new_node->set_backward(i, update[i]);
    }
    else
      break; // 某一层不需要更新，则停止更新更高层
  }

  current_level = new_level; // 更新当前的层数
}

void SkipList::remove(const std::string &key)
{
  std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);

  //   标记当前的节点
  auto current = head;

  for (int i = current->forward.size() - 1; i >= 0; i--)
  {
    while (current->forward[i] && current->forward[i]->key < key)
    {
      current = current->forward[i];
    }
    update[i] = current;
  }

  // 移动到最底层
  current = current->forward[0];

  //   如果找到目标节点, 执行删除操作
  if (current && current->key == key)
  {
    // 记录要更新的节点
    for (int i = 0; i < current_level; i++)
    {
      if (update[i]->forward[i] != current)
      {
        break;
      }
      update[i]->forward[i] = current->forward[i];
    }
  }

  // 更新 backward 指针
  for (int i = 0; i < current->backward.size() && i < current_level; ++i)
  {
    if (current->forward[i])
    {
      current->forward[i]->set_backward(i, update[i]);
    }
  }

  //   更新内存大小
  size_bytes -= key.size() + current->value.size();

  // 如果我们的删除节点是最高层的节点, 需要更新跳表的当前层级
  while (current_level > 1 && head->forward[current_level - 1] == nullptr)
  {
    current_level--;
  }
}

std::optional<std::string> SkipList::get(const std::string &key)
{
  auto current = head;

  // 从高层进行查找
  for (int i = current->forward.size() - 1; i >= 0; i--)
  {
    while (current->forward[i] && current->forward[i]->key < key)
    {
      current = current->forward[i];
    }
  }

  // 移动到最底层
  current = current->forward[0];
  if (current && current->key == key)
  {
    return current->value;
  }
  else
  {
    return std::nullopt;
  }

  return std::nullopt;
}

void SkipList::clear()
{
  head = std::make_shared<SkipListNode>("", "", max_level);
  size_bytes = 0;
}

SkipListIterator SkipList::begin()
{
  return SkipListIterator(head->forward[0]);
}

SkipListIterator SkipList::end()
{
  return SkipListIterator(nullptr);
}

size_t SkipList::get_size() const
{
  return size_bytes;
}

std::vector<std::pair<std::string, std::string>> SkipList::flush()
{
  std::vector<std::pair<std::string, std::string>> res;
  auto node = head->forward[0];

  while (node)
  {
    res.emplace_back(node->key, node->value);
    node = node->forward[0];
  }
  return res;
}

/*********************** SkipListIterator *******************/
SkipListIterator &SkipListIterator::operator++()
{
  if (current)
  {
    current = current->forward[0];
  }
  return *this;
}

SkipListIterator SkipListIterator::operator++(int)
{
  SkipListIterator temp = *this;
  ++*this;
  return temp;
}

bool SkipListIterator::operator==(const SkipListIterator &other) const
{
  return current == other.current;
}
bool SkipListIterator::operator!=(const SkipListIterator &other) const
{
  return current != other.current;
}

std::string SkipListIterator::get_key() const
{
  return current->key;
}
std::string SkipListIterator::get_value() const
{
  return current->value;
}

bool SkipListIterator::is_valid() const
{
  return current != nullptr;
}

bool SkipListIterator::is_end() const
{
  return current == nullptr;
}

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