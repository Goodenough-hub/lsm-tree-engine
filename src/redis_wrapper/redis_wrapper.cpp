#include "../../include/redis_wrapper/redis_wrapper.h"
#include "../../include/const.h"

#include <chrono> // 提供高精度时间库（如std::chrono::time_point），用于实现过期时间计算
#include <ctime>  // 支持C风格时间处理（如std::time_t），用于时间戳转换
#include <mutex>  // 实现互斥锁(std::mutex)和读写锁(std::shared_mutex)，保障线程安全
#include <shared_mutex>
#include <optional> // 定义std::optional容器，用于处理可能为空的返回值（如键不存在场景）
#include <string>
#include <sstream>

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
std::string get_hash_filed_key(const std::string &key, const std::string &filed)
{
    return std::string(REDIS_HASH_HEADER) + key + "_" + filed;
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