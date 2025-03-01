#include "../../include/skiplist/skiplist.h"
#include <cstddef>
#include <stdexcept>
#include <tuple>

/*********************** SkipList *******************/

// 构造函数
SkipList::SkipList(int max_level) {
  this->max_level = max_level;
  this->head = std::make_shared<SkipListNode>("", "", max_level); // 创建头节点
  this->current_level = 1;

  // 随机数
  this->dis_01 = std::uniform_int_distribution<>(0, 1);
  this->dis_level = std::uniform_int_distribution<>(0, (1 << max_level) - 1);
  this->gen = std::mt19937(this->rd());
}

int SkipList::random_level() {
  int level = 1;
  // 通过抛硬币的方式生成随机层数
  // 每一次抛硬币都有50%的概率增加一层
  while (dis_01(gen) && level < max_level) { // 执行次数：[0, max_level - 1]
    level++;
  }
  return level; // [1, max_level]
}

void SkipList::put(const std::string &key, const std::string &value) {
//   if (value.empty()) {
//     throw std::runtime_error("value cannot be empty"); // 值为空，抛出异常
//   }

  //   标记当前的节点
  //   初始化当前节点为头节点 head，用于遍历跳表
  auto current = head;

  // 创建一个大小为 max_level 的向量 update，用于记录每一层的前驱节点。
  std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);

  // 从最高层开始向下遍历，找到每一层中小于给定 key 的最大节点，并将其记录到 update 中。
  for (int i = current_level - 1; i >= 0; i--) {
    while (current->forward[i] && current->forward[i]->key < key) {
      current = current->forward[i];
    }
    update[i] = current;
  }

  // 移动到最底层
  current = current->forward[0];

  // 判断 key 是否存在
  if (current && current->key == key) {
    size_bytes += value.size() - current->value.size();
    current->value = value;
    return;
  }

  //   value不存在则需要创建
  int new_level = std::max(random_level(), current_level);
  for (int i = current_level; i < new_level; ++i) {
    update[i] = head;
  }

  auto new_node = std::make_shared<SkipListNode>(key, value, new_level);
  size_bytes += new_node->key.size() + new_node->value.size();

  // 三种状态的update
  // 第0层一定更新
  // 如果创建了新的层级，所有层级都更新
  // 每次50%的概率更新，某层不更新则中断更新
  int radn_bits = dis_level(gen);
  for (int i = 0; i < new_level; ++i) {
    bool is_update = false;
    if(i== 0 || (new_level > current_level) || (radn_bits & (1 << i)))
    {
      is_update = true; // 需要更新
    }
    if(is_update)
    {
      // 更新forward与backward
      new_node->forward[i] = update[i]->forward[i];
      if(new_node->forward[i])
      {
        new_node->forward[i]->set_backward(i, new_node);
      }
      
      update[i]->forward[i] = new_node;
      new_node->set_backward(i, update[i]);
    }
    else break; // 某一层不需要更新，则停止更新更高层
  }

  current_level = new_level; // 更新当前的层数
}

void SkipList::remove(const std::string &key) {
    std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);
  
    //   标记当前的节点
    auto current = head;
  
    for (int i = current->forward.size() - 1; i >= 0; i--) {
      while (current->forward[i] && current->forward[i]->key < key) {
        current = current->forward[i];
      }
      update[i] = current;
    }
  
    // 移动到最底层
    current = current->forward[0];
  
    //   如果找到目标节点, 执行删除操作
    if (current && current->key == key) {
      // 记录要更新的节点
      for (int i = 0; i < current_level; i++) {
        if (update[i]->forward[i] != current) {
          break;
        }
        update[i]->forward[i] = current->forward[i];
      }
    }
  
    // 更新 backward 指针
    for (int i = 0; i < current->backward.size() && i < current_level; ++i) {
      if (current->forward[i]) {
        current->forward[i]->set_backward(i, update[i]);
      }
    }
  
    //   更新内存大小
    size_bytes -= key.size() + current->value.size();
  
    // 如果我们的删除节点是最高层的节点, 需要更新跳表的当前层级
    while (current_level > 1 && head->forward[current_level - 1] == nullptr) {
      current_level--;
    }
}
  
std::optional<std::string> SkipList::get(const std::string &key) {
    auto current = head;

    // 从高层进行查找
    for (int i = current->forward.size() - 1; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->key < key) {
        current = current->forward[i];
        }
    }

    // 移动到最底层
    current = current->forward[0];
    if (current && current->key == key) {
        return current->value;
    } else {
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

/*********************** SkipListIterator *******************/
SkipListIterator& SkipListIterator::operator++()
{
    if(current) 
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

