#pragma once

#include <string>
#include <vector>

class BloomFilter
{
private:
    size_t expected_elements_;   // 预期存储的元素数量
    double false_positive_rate_; // 允许的假阳率（0.0~1.0）

    // 存储结构
    std::vector<bool> bits_; // 位数组，存储布隆过滤器中的每个位，二进制状态。高效的空间利用

    // 自动计算的哈希参数
    size_t num_hashes_; // 哈希函数的数量
    size_t num_bits_;   // 位数组的总长度

private:
    size_t hash1(const std::string &key) const;
    size_t hash2(const std::string &key) const;

    size_t hash(const std::string &key, size_t idx) const;

public:
    BloomFilter();

    // 主构造函数
    BloomFilter(size_t expected_elements, double false_positive_rate);

    // 带位数的特殊构造，用于反序列化的场景
    BloomFilter(size_t expected_elements, double false_positive_rate, size_t num_bits);

    void add(const std::string &key);                     // 添加元素到布隆过滤器中
    bool possibly_contains(const std::string &key) const; // 判断布隆过滤器中是否存在某个元素

    std::vector<uint8_t> encode(); // 序列号化数组为字节流（用于持久化存储）

    static BloomFilter decode(std::vector<uint8_t> &data); // 反序列化静态方法（从字节流重建过滤器对象）
};