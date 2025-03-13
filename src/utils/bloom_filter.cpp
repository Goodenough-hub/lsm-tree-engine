#include "../../include/utils/bloom_filter.h"
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

// 默认构造函数，仅用于反序列化场景
BloomFilter::BloomFilter() {}

// 主构造函数，根据预期元素数量和假阳率构造布隆过滤器
BloomFilter::BloomFilter(size_t expected_elements, double false_positive_rate)
    : expected_elements_(expected_elements), false_positive_rate_(false_positive_rate)
{
    // 通过假阳率和预期元素的个数计算哈希函数个数和bits_长度
    // 根据布隆过滤器公式计算所需位数m
    double m = -static_cast<double>(expected_elements) *
               std::log(false_positive_rate_) / std::pow(std::log(2), 2);
    num_bits_ = static_cast<size_t>(std::ceil(m)); // 向上取整保证精度

    // 计算哈希函数的数量
    num_hashes_ = static_cast<size_t>(std::ceil(std::log(2) * num_bits_ / expected_elements));
    bits_.resize(num_bits_, false); // 初始化位数组
}

// 添加元素到布隆过滤器
void BloomFilter::add(const std::string &key)
{
    // 计算哈希值对应的位索引
    // 双重哈希法，通过组合两个基础哈希生成多个哈希值
    for (size_t i = 0; i < num_hashes_; i++)
    {
        bits_[hash(key, i)] = true; // 标记对应位为true
    }
}

// 基础哈希函数1（使用标准哈希）
size_t BloomFilter::hash1(const std::string &key) const
{
    std::hash<std::string> hasher; // 标准字符串哈希器
    return hasher(key);            // 返回哈希值
}

// 基础哈希函数2（添加"salt"扰动）
size_t BloomFilter::hash2(const std::string &key) const
{
    std::hash<std::string> hasher;
    return hasher(key + "salt"); // 通过添加后缀生成不同哈希值
}

// 复合哈希函数（生成第idx个哈希值）
size_t BloomFilter::hash(const std::string &key, size_t idx) const
{
    auto h1 = hash1(key);
    auto h2 = hash2(key);

    // 线性组合公式：(h1 + i*h2) mod num_bits_
    return (h1 + idx * h2) % num_bits_;
}

// 序列化方法（将对象转换为字节流）
std::vector<uint8_t> BloomFilter::encode()
{
    std::vector<uint8_t> data; // 存储序列化结果

    // 序列化元数据（二进制直接拷贝）
    // 编码expected_elements_
    data.insert(data.end(),
                reinterpret_cast<const uint8_t *>(&expected_elements_),
                reinterpret_cast<const uint8_t *>(&expected_elements_) +
                    sizeof(expected_elements_));

    // 编码false_positive_rate_
    data.insert(data.end(),
                reinterpret_cast<const uint8_t *>(&false_positive_rate_),
                reinterpret_cast<const uint8_t *>(&false_positive_rate_) +
                    sizeof(false_positive_rate_));
    // 编码num_bits_
    data.insert(data.end(),
                reinterpret_cast<const uint8_t *>(&num_bits_),
                reinterpret_cast<const uint8_t *>(&num_bits_) +
                    sizeof(num_bits_));
    // 编码num_hashes_
    data.insert(data.end(),
                reinterpret_cast<const uint8_t *>(&num_hashes_),
                reinterpret_cast<const uint8_t *>(&num_hashes_) +
                    sizeof(num_hashes_));

    // 编码位数组（压缩存储）
    // 编码bits
    size_t numn_bytes = (num_bits_ + 7) / 8; // 计算需要的字节数（向上取整）
    for (size_t i = 0; i < numn_bytes; i++)
    {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8; j++) // 每8位压缩为1字节
        {
            if (i * 8 + j < num_bits_) // 防止越界
            {
                byte |= bits_[i * 8 + j] << j; // 位操作设置对应位
            }
        }
        data.push_back(byte); // 添加压缩后的字节
    }
    return data;
}

// 反序列化静态方法（从字节流重建对象）
BloomFilter BloomFilter::decode(std::vector<uint8_t> &data)
{
    size_t idx = 0; // 数据读取指针

    // 解码 expected_elements_
    size_t expected_elements;
    std::memcpy(&expected_elements, &data[idx], sizeof(expected_elements));
    idx += sizeof(expected_elements);

    // 解码false_positive_rate_
    double false_positive_rate;
    std::memcpy(&false_positive_rate, &data[idx], sizeof(false_positive_rate));
    idx += sizeof(false_positive_rate);

    // 解码num_bits_
    size_t num_bits;
    std::memcpy(&num_bits, &data[idx], sizeof(num_bits));
    idx += sizeof(num_bits);

    // 解码num_hashes
    size_t num_hashes;
    std::memcpy(&num_hashes, &data[idx], sizeof(num_hashes));
    idx += sizeof(num_hashes);

    // 解码bits_
    // 解码位数组
    std::vector<bool> bits(num_bits, false);
    size_t num_bytes = (num_bits + 7) / 8;

    for (size_t i = 0; i < num_bytes; i++)
    {
        uint8_t byte = data[idx++];
        for (size_t j = 0; j < 8; j++) // 解压字节到位数组
        {
            if (i * 8 + j < num_bits) // 防止越界
            {
                bits[i * 8 + j] = (byte >> j) & 1; // 提取每一位
            }
        }
    }

    // 构建对象
    BloomFilter bf;
    bf.expected_elements_ = expected_elements;
    bf.false_positive_rate_ = false_positive_rate;
    bf.num_bits_ = num_bits;
    bf.num_hashes_ = num_hashes;
    bf.bits_ = bits;

    return bf;
}