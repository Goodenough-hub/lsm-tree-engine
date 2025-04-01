#include "../../include/redis_wrapper/redis_wrapper.h"
#include "../../include/const.h"

#include <chrono> // 提供高精度时间库（如std::chrono::time_point），用于实现过期时间计算
#include <ctime>  // 支持C风格时间处理（如std::time_t），用于时间戳转换
#include <mutex>  // 实现互斥锁(std::mutex)和读写锁(std::shared_mutex)，保障线程安全
#include <shared_mutex>
#include <optional> // 定义std::optional容器，用于处理可能为空的返回值（如键不存在场景）
#include <string>
#include <sstream>
#include <iomanip>

// 正常存储：key， value
// 同时存储过期时间：expire_key， expire_time
std::string get_expire_key(const std::string &key)
{
    return std::string(REDIS_EXPIRE_HEADER) + "_" + key;
}

std::string get_expire_time(const std::string &second_count)
{
    // 从当前时间戳加上指定的过期时间长度
    auto now = std::chrono::system_clock::now();                 // 获取当前时间戳
    auto now_time_t = std::chrono::system_clock::to_time_t(now); // 将当前时间点转换为 time_t 类型的时间戳 now_time_t
    auto time_add = std::stoll(second_count);                    // 将输入的秒数字符串转换为长整型 time_add
    auto expire_time = std::to_string(now_time_t + time_add);    // 计算过期时间戳 now_time_t + time_add，并将其转换为字符串返回。
    return expire_time;
}

// 小key加上前缀
inline std::string get_hash_filed_key(const std::string &key, const std::string &filed)
{
    return std::string(REDIS_HASH_HEADER) + key + "_" + filed;
}

inline std::string get_zset_key_preffix(const std::string &key)
{
    return REDIS_SORTED_SET_PREFIX + key + "_";
}

inline std::string get_set_key_preffix(const std::string &key)
{
    return REDIS_SET_PREFIX + key + "_";
}

inline std::string get_set_elem_key(const std::string &key, const std::string &elem)
{
    return REDIS_SET_PREFIX + key + "_SCORE_" + elem;
}

inline std::string get_zset_score_preffix(const std::string &key)
{
    return REDIS_SORTED_SET_PREFIX + key + "_SCORE_";
}

inline std::string get_zset_key_score(const std::string &key, const std::string &score)
{
    // 50 < 100
    // '50' > '100'
    // '050' < '100'
    std::ostringstream oss;
    oss << std::setw(REDIS_SORTED_SET_SCORE_LEN) << std::setfill('0') << score;
    std::string formatted_score = oss.str();

    std::string res = get_zset_score_preffix(key) + formatted_score;
    return res;
}

std::string get_zset_key_elem(const std::string &key, const std::string &elem)
{
    return get_zset_key_preffix(key) + elem;
}

std::string get_zset_score_item(const std::string &key)
{
    const std::string score_preffix = "_SCORE_";

    size_t score_pos = key.find(score_preffix);

    if (score_pos != std::string::npos) // 未找到位置
    {
        return key.substr(score_pos + score_preffix.size());
    }
    else
    {
        return "";
    }
}

// 去除大key的前缀，获得所有小key，放入数组
std::vector<std::string> get_fileds_from_hash_value(const std::optional<std::string> &filed_list_opt)
{
    std::string field_list = filed_list_opt.value_or("");
    if (!field_list.empty())
    {
        // 去除前缀
        std::string prefix = REDIS_HASH_VALUE_PREFIX;
        field_list = field_list.substr(prefix.size(), field_list.size() - prefix.size());
    }
    std::vector<std::string> fields;

    // f1$f2$f3
    // 使用 REDIS_FILED_SEPARATOR 分隔字符串，将每个分隔后的字段存入向量 fields。
    std::istringstream iss(field_list);
    std::string token;
    while (std::getline(iss, token, REDIS_FILED_SEPARATOR))
    {
        fields.push_back(token);
    }
    return fields;
}

// 从数组中读取小key的值，加入分割符，前缀，放入string中。
std::string get_hash_value_from_fields(const std::vector<std::string> &fields)
{
    std::ostringstream oss;
    oss << REDIS_HASH_VALUE_PREFIX;
    for (size_t i = 0; i < fields.size(); i++)
    {
        if (i > 0)
        {
            oss << REDIS_FILED_SEPARATOR;
        }
        oss << fields[i];
    }
    return oss.str();
}

RedisWrapper::RedisWrapper(const std::string &path)
{
    lsm = std::make_unique<LSMEngine>(path);
}

bool is_expired(const std::optional<std::string> &expire_str, std::time_t *now_time)
{
    if (!expire_str.has_value())
    {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    if (now_time != nullptr)
    {
        *now_time = now_time_t;
    }
    return std::stol(expire_str.value()) < now_time_t;
}

std::string RedisWrapper::set(std::vector<std::string> &args)
{
    std::unique_lock<std::shared_mutex> lock(redis_mtx); // 上写锁
    this->lsm->put(args[1], args[2]);
    std::string expire_key = get_expire_key(args[1]);
    if (lsm->get(expire_key).has_value())
    {
        lsm->remove(expire_key);
    }
    return "+OK\r\n";
}

std::string RedisWrapper::get(std::vector<std::string> &args)
{
    std::shared_lock<std::shared_mutex> rlock(redis_mtx); // 上读锁
    auto key_query = lsm->get(args[1]);

    std::string expire_key = get_expire_key(args[1]);
    auto expire_query = lsm->get(expire_key);

    if (key_query.has_value())
    {
        // 检查TTL
        if (expire_query.has_value())
        {
            if (is_expired(expire_query, nullptr))
            {
                rlock.unlock();
                std::unique_lock<std::shared_mutex> wlock(redis_mtx);
                lsm->remove(args[1]);
                lsm->remove(expire_key);
                return "$-1\r\n";
            }
            else
            {
                return "$" + std::to_string(key_query.value().size()) + "\r\n" + key_query.value() + "\r\n";
            }
        }
        else
        {
            return "$" + std::to_string(key_query.value().size()) + "\r\n" + key_query.value() + "\r\n";
        }
    }
    else
    {
        return "$-1\r\n";
    }
}

std::string RedisWrapper::expire(std::vector<std::string> &args)
{
    std::unique_lock<std::shared_mutex> lock(redis_mtx);
    std::string expire_key = get_expire_key(args[1]);
    auto expire_time = get_expire_time(args[2]);
    lsm->put(expire_key, expire_time); // 存储需要设置过期的key和对应的过期时间
    return ":1\r\n";                   // 返回成功标志字符串
}

// ttl key1
std::string RedisWrapper::ttl(std::vector<std::string> &args)
{
    std::shared_lock<std::shared_mutex> lock(redis_mtx);
    std::string expire_key = get_expire_key(args[1]);

    auto key_query = lsm->get(args[1]);
    auto expire_query = lsm->get(expire_key);

    if (key_query.has_value())
    {
        if (expire_query.has_value())
        {
            std::time_t now_time;
            if (is_expired(expire_query.value(), &now_time))
            {
                // 过期了，key不存在
                // 过期了也不在这里删除，因为TTL设置为只读，
                // 删除在之后的查询key的时候惰性执行
                return ":-2\r\n"; // -2表示key过期了
            }
            else
            {
                // 没有过期
                auto now = std::chrono::system_clock::now();
                return ":" + std::to_string(std::stoll(expire_query.value()) - now_time) + "\r\n";
            }
        }
        else
        {
            // 没有设置过期时间
            return ":-1\r\n";
        }
    }
    else
    {
        // key不存在
        return ":1\r\n";
    }
}

// 每个filed单独存一份
// 整个大key存一些元信息
// 大key: s1
// filed:
//       name: zhangsan
//       age: 18
//       sex: male
//
// s1, REDIS_HASH_VALUE_PREFFIX_$REDIS_HASH_HEADER_s1_name$REDIS_HASH_HEADER_s1_age$
// REDIS_HASH_HEADER_s1_name, zhangsan
// REDIS_HASH_HEADER_s1_age, 18
// REDIS_HASH_HEADER_s1_sex, male
//
// hset s1 name zhangsan
//
std::string RedisWrapper::hset(std::vector<std::string> &args)
{
    std::shared_lock<std::shared_mutex> rlock(redis_mtx); // 上读锁

    bool is_expired = expire_hash_clean(args[1], rlock); // 判断是否过期

    if (!is_expired) // 没有过期
    {
        rlock.unlock(); // 解锁读锁
    }
    std::unique_lock<std::shared_mutex> wlock(redis_mtx); // 上写锁

    // 更新字段 小key字段
    std::string field_key = get_hash_filed_key(args[1], args[2]);
    lsm->put(field_key, args[3]);

    // 更新字段列表，大key字段
    auto field_list_opt = lsm->get(args[1]);
    auto field_list = get_fileds_from_hash_value(field_list_opt); // 获取所有小key的字段

    if (std::find(field_list.begin(), field_list.end(), args[2]) == field_list.end())
    {
        field_list.push_back(args[2]);
        auto new_value = get_hash_value_from_fields(field_list);
        lsm->put(args[1], new_value);
    }

    return "+OK\r\n";
}

std::string RedisWrapper::hget(std::vector<std::string> &args)
{
    std::shared_lock<std::shared_mutex> rlock(redis_mtx); // 上写锁

    bool is_expire = expire_hash_clean(args[1], rlock); // 判断是否过期，过期就清理数据

    if (is_expire) // 过期了
    {
        return "$-1\r\n";
    }

    std::string field_key = get_hash_filed_key(args[1], args[2]); // 小key加前缀
    auto value_opt = lsm->get(field_key);                         // 找对应的value

    if (value_opt.has_value()) // value存在
    {
        return "$" + std::to_string(value_opt.value().size()) + "\r\n" + value_opt.value() + "\r\n";
    }
    else
    {
        return "$-1\r\n"; // 没有找到对应的value
    }
}

// 检查指定的Redis哈希值是否过期，如果过期则清理数据
// 查大key
bool RedisWrapper::expire_hash_clean(const std::string &key, std::shared_lock<std::shared_mutex> &rlock)
{
    std::string expire_key = get_expire_key(key); // 拼接形成expire_key
    auto expire_query = lsm->get(expire_key);     // 在lsm中找是否有expire_key

    if (is_expired(expire_query, nullptr)) // 过期了
    {
        rlock.unlock();                                          // 解锁读锁
        std::unique_lock<std::shared_mutex> wlock(redis_mtx);    // 上写锁
        auto fields = get_fileds_from_hash_value(lsm->get(key)); // 获取所有小的key值
        for (const auto &field_key : fields)                     // 移除所有小key
        {
            lsm->remove(field_key);
        }
        lsm->remove(key);        // 移除大key
        lsm->remove(expire_key); // 移除过期的key
        return true;
    }
    return false;
}

// ZADD z1 95 math
// ZADD z1 100 English

// 大key：(Z1, xxx)
// 成员的存储：每一个成员存储两次
// 前缀标记了这个键值对是 有序集合的成员
// 第一次存储的是(prefix_z1_ELEM_xxx, score)
// 第二次存储的是(prefix_z1_SCORE_score_yyyy, member)
std::string RedisWrapper::zadd(std::vector<std::string> &args)
{
    std::shared_lock<std::shared_mutex> rlock(redis_mtx);

    auto key = args[1];
    bool is_expired = expire_zset_clean(key, rlock); // 检查键是否过期。如果过期，则清理相关数据。

    if (!is_expired)
    {
        rlock.unlock(); // 释放读锁
    }

    std::unique_lock<std::shared_mutex> wlock(redis_mtx); // 上写锁

    std::vector<std::pair<std::string, std::string>> put_kvs; // 存储待插入的键值对

    auto value = get_zset_key_preffix(key); // 直接将前缀作为value
    if (!lsm->get(value).has_value())
    {
        // 大key不存在
        put_kvs.emplace_back(key, value); // 插入格式：<大键名，大键前缀>
    }

    std::vector<std::string> remove_keys; // 删除容器
    int added_count = 0;                  // 记录成功添加的成员数量。

    // 遍历成员和分数
    for (size_t i = 2; i < args.size(); i += 2)
    {
        // 从 args 中依次提取分数（score）和成员（elem）。
        std::string score = args[i];                            // 分数
        std::string elem = args[i + 1];                         // 成员
        std::string key_score = get_zset_key_score(key, score); // 分数键（格式：ZSET_z1_SCORE_00095）
        std::string key_elem = get_zset_key_elem(key, elem);    // 成员键（格式：ZSET_z1_ELEM_math）

        auto query_elem = lsm->get(key_elem);
        // 检查成员是否存在
        if (query_elem.has_value())
        {
            // 将以前的旧记录删除
            std::string original_score = query_elem.value();
            if (original_score == score)
            {
                // 分数未发生变化，不需要更新
                continue;
            }

            // 需要移除
            // 生成旧分数键并加入删除列表
            std::string original_key_score = get_zset_key_score(key, original_score);
            remove_keys.emplace_back(original_key_score); // 格式：ZSET_z1_SCORE_历史分数
        }

        put_kvs.emplace_back(key_score, elem); // 存储格式：<分数键, 成员>
        put_kvs.emplace_back(key_elem, score); // 存储格式：<成员键, 分数>
        added_count++;                         // 更新成功计数器
    }

    // 批量删除和插入操作
    lsm->remove_batch(remove_keys); // 删除旧分数记录
    lsm->put_batch(put_kvs);        // 插入新记录

    return ":" + std::to_string(added_count) + "\r\n"; // 返回成功添加的成员数量
}

std::string RedisWrapper::zrange(std::vector<std::string> &args)
{
    std::string key = args[1];      // 有序集合键名
    int start = std::stoi(args[2]); // 起始索引（支持负数，如-1表示最后一个元素）
    int end = std::stoi(args[3]);   // 结束索引

    std::shared_lock<std::shared_mutex> rlock(redis_mtx); // 上读锁

    // 过期检查：调用专用有序集合过期清理方法
    bool is_expired = expire_zset_clean(key, rlock);

    if (is_expired)
    {
        return "*0\r\n"; // 返回空集合响应
    }

    // 谓词查询
    std::string preffix_score = get_zset_score_preffix(key);
    // 执行范围查询：获取所有以prefix_score开头的键值对（有序存储）
    auto result = lsm->iter_monotony_predicate([&preffix_score](const std::string &elem)
                                               { return -elem.compare(0, preffix_score.size(), preffix_score); }); // 通过负数比较实现升序排序

    if (!result.has_value()) // 空结果
    {
        return "*0\r\n";
    }

    // 解析查询结果
    std::vector<std::pair<std::string, std::string>> elements;
    auto [elem_begin, elem_end] = result.value();
    for (; elem_begin != elem_end; ++elem_begin)
    {
        std::string key_score = elem_begin->first;          // 分数键（格式：ZSET_z1_SCORE_00095）
        std::string elem = elem_begin->second;              // 成员值
        std::string score = get_zset_score_item(key_score); // 从键名提取分数（"00095" -> "95"）
        elements.emplace_back(std::make_pair(elem, score)); // 存储为 <成员, 原始分数>
    }

    // 处理负数索引（Redis兼容逻辑）
    if (start < 0)
    {
        start += elements.size();
    }
    if (end < 0)
    {
        end += elements.size();
    }

    // 索引边界修正
    if (end >= elements.size())
    {
        end = elements.size() - 1;
    }

    // 输入不合法
    if (start > end)
    {
        return "*0\r\n";
    }

    std::ostringstream oss;
    oss << "*" << end - start + 1 << "\r\n"; // 响应头（元素数量）
    for (int i = start; i <= end; ++i)
    {
        // 按协议格式输出每个元素（当前输出分数值，若需返回成员需改为elements[i].first）
        oss << "$" << elements[i].second.size() << "\r\n" // 值长度
            << elements[i].second << "\r\n";              // 值内容（分数）
    }
    return oss.str();
}

bool RedisWrapper::expire_zset_clean(const std::string &key, std::shared_lock<std::shared_mutex> &rlock)
{
    std::string expire_key = get_expire_key(key); // 生成过期键名
    auto expire_query = lsm->get(expire_key);     // 查询过期时间

    if (is_expired(expire_query, nullptr))
    {
        // 过期了
        rlock.unlock();                                       // 释放读锁
        std::unique_lock<std::shared_mutex> wlock(redis_mtx); // 上写锁

        lsm->remove(key);        // 大key（如 ZSET_z1）
        lsm->remove(expire_key); // 删除过期键（如 expire_z1）

        auto preffix = get_zset_key_preffix(key); // 生成有序集合成员键前缀（格式示例：ZSET_z1_）

        // 查询所有以该前缀开头的键（成员键和分数键）
        auto result_elem = lsm->iter_monotony_predicate([&preffix](const std::string &elem)
                                                        { return -elem.compare(0, preffix.size(), preffix); });
        if (result_elem.has_value())
        {
            // 如果存在匹配的键
            // 获取键范围迭代器
            auto [elem_begin, elem_end] = result_elem.value();
            std::vector<std::string> remove_vec;

            // 遍历所有匹配的键
            for (; elem_begin != elem_end; ++elem_begin)
            {
                // 收集待删除的键名
                remove_vec.push_back(elem_begin->first);
            }
            lsm->remove_batch(remove_vec); // 批量删除
        }
        return true;
    }
    return false;
}

std::string RedisWrapper::sadd(std::vector<std::string> &args)
{
    std::string key = args[1];
    std::shared_lock<std::shared_mutex> rlock(redis_mtx);

    bool is_expired = expire_set_clean(key, rlock);

    if (!is_expired)
    {
        rlock.unlock();
    }

    std::vector<std::pair<std::string, std::string>> put_kvs;

    // 获取写锁
    for (size_t i = 2; i < args.size(); i++)
    {
        std::string elem = args[i];
        std::string elem_key = get_set_elem_key(key, elem);

        if (!lsm->get(elem_key).has_value())
        {
            put_kvs.emplace_back(elem_key, "1");
        }
    }

    // 更新集合大小
    auto key_query = lsm->get(key);
    int set_size = put_kvs.size();
    if (key_query.has_value())
    {
        auto prev_size = std::stoi(key_query.value());
        set_size += prev_size;
    }
    put_kvs.emplace_back(key, std::to_string(set_size));

    lsm->put_batch(put_kvs);

    return ":" + std::to_string(put_kvs.size() - 1) + "\r\n";
}

std::string RedisWrapper::srem(std::vector<std::string> &args)
{
    std::string key = args[1];
    std::shared_lock<std::shared_mutex> rlock(redis_mtx);

    bool is_expired = expire_set_clean(key, rlock);

    if (is_expired) // 过期了
    {
        return ":0\r\n";
    }

    rlock.unlock();

    std::vector<std::string> del_keys;

    // 获取写锁
    std::unique_lock<std::shared_mutex> wlock(redis_mtx);

    for (size_t i = 2; i < args.size(); i++)
    {
        std::string elem = args[i];
        std::string elem_key = get_set_elem_key(key, elem);

        if (!lsm->get(elem_key).has_value())
        {
            del_keys.emplace_back(elem_key);
        }
    }

    // 更新集合大小
    auto key_query = lsm->get(key);
    int set_size = -del_keys.size();
    if (key_query.has_value())
    {
        auto prev_size = std::stoi(key_query.value());
        set_size += prev_size;
    }

    lsm->put(key, std::to_string(set_size));
    lsm->remove_batch(del_keys);

    return ":" + std::to_string(del_keys.size()) + "\r\n";
}

std::string RedisWrapper::sismember(std::vector<std::string> &args)
{
    std::string key = args[1];

    std::shared_lock<std::shared_mutex> rlock(redis_mtx);

    bool is_expired = expire_set_clean(key, rlock);

    if (is_expired) // 过期了
    {
        return ":0\r\n";
    }

    std::string elem_key = get_set_elem_key(key, args[2]);
    if (lsm->get(elem_key).has_value())
    {
        return ":1\r\n";
    }
    else
    {
        return ":0\r\n";
    }
}

bool RedisWrapper::expire_set_clean(const std::string &key, std::shared_lock<std::shared_mutex> &rlock)
{
    std::string expire_key = get_expire_key(key);
    auto expire_query = lsm->get(expire_key);

    if (is_expired(expire_query, nullptr))
    {
        rlock.unlock();
        std::unique_lock<std::shared_mutex> wlock(redis_mtx);

        lsm->remove(key); // 大key
        lsm->remove(expire_key);

        auto preffix = get_set_key_preffix(key);
        auto result_elem = lsm->iter_monotony_predicate([&preffix](const std::string &elem)
                                                        { return -elem.compare(0, preffix.size(), preffix); });
        if (result_elem.has_value())
        {
            auto [elem_begin, elem_end] = result_elem.value();
            for (; elem_begin != elem_end; ++elem_begin)
            {
                remove_vec.push_back(elem_begin->first);
            }
        }
    }
}