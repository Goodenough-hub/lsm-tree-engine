#pragma once
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <shared_mutex>
#include <cstdlib>
#include <random>
#include <functional>

// 跳表的节点
struct SkipListNode
{
    std::string key;   // 节点存储的键
    std::string value; // 节点存储的值
    std::vector<std::shared_ptr<SkipListNode>>
        forward; // 指向不同层级的下一个节点的指针数组，shared_ptr是为了在多层级中保持对下一个节点的强引用，确保节点在有引用时不会被销毁。
    std::vector<std::weak_ptr<SkipListNode>>
        backward; // 指向不同层级的下一个节点的指针数组，weak_ptr 是为了避免双向链表中出现循环引用导致内存泄漏，同时允许反向遍历时获取节点。
    SkipListNode(const std::string &k, const std::string &v, int level)
        : key(k), value(v), forward(level, nullptr),
          backward(level, std::weak_ptr<SkipListNode>()) {}
    void set_backward(int level, std::shared_ptr<SkipListNode> node)
    {
        backward[level] = std::weak_ptr<SkipListNode>(node);
    }
};

class SkipListIterator;
class SkipList
{
private:
    std::shared_ptr<SkipListNode> head; // 跳表的头节点
    int max_level;                      // 跳表的最大层数
    int current_level;                  // 当前跳表的层数
    size_t size_bytes = 0;

    std::random_device rd;
    std::uniform_int_distribution<> dis_01;
    std::uniform_int_distribution<> dis_level;
    std::mt19937 gen;

    int random_level(); // 随机生成节点的层数

public:
    SkipList(int max_level = 16); // 默认的最大层数为16
    void put(const std::string &key, const std::string &value);
    std::optional<std::string> get(const std::string &key);
    void remove(const std::string &key);
    void clear();

    // begin() 和 end() 迭代器
    SkipListIterator begin();
    SkipListIterator end();

    // 基于传入的谓词返回一对迭代器，表示满足该谓词条件的范围
    // 允许用户自定义筛选逻辑，增强了 SkipList 的灵活性和通用性
    std::optional<std::pair<SkipListIterator, SkipListIterator>> iters_monotony_predicate(std::function<int(const std::string &)> predicate);

    size_t get_size() const;

    std::vector<std::pair<std::string, std::string>> flush(); // 获取键值对
};

class SkipListIterator
{
private:
    std::shared_ptr<SkipListNode> current;

public:
    SkipListIterator(std::shared_ptr<SkipListNode> node) : current(node) {};

    SkipListIterator &operator++();   // 前置自增
    SkipListIterator operator++(int); // 后置自增
    bool operator==(const SkipListIterator &other) const;
    bool operator!=(const SkipListIterator &other) const;

    std::string get_key() const;
    std::string get_value() const;

    bool is_valid() const;
    bool is_end() const;
};