#pragma once
#include "../../include/redis_wrapper/redis_wrapper.h"
#include <cstring>

enum class OPS
{
    // 测试
    PING,
    // IO操作
    FLUSHALL,
    SAVE,
    // 基础操作
    SET,
    GET,
    DEL,
    INCR,
    DECR,
    EXPIRE,
    TTL,
    // 哈希操作
    HGET,
    HSET,
    HDEL,
    HKEYS,
    // 链表操作
    LPUSH,
    RPUSH,
    LPOP,
    RPOP,
    LLEN,
    LRANGE,
    // 有序集合操作
    ZADD,
    ZCARD,
    ZREM,
    ZSCORE,
    ZINCRBY,
    ZRANK,
    ZRANGE,
    // 集合操作
    SADD,
    SREM,
    SISMEMBER,
    SCARD,
    SMEMBERS,
    // 其他
    UNKNOWN
};

OPS string2OPS(const std::string &opStr);

std::string flushall_handler(RedisWrapper &engine);
std::string save_handler(RedisWrapper &engine);

// 基础操作
std::string set_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string get_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string del_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string incr_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string decr_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string expire_handler(std::vector<std::string> &args,
                           RedisWrapper &engine);
std::string ttl_handler(std::vector<std::string> &args, RedisWrapper &engine);

// 哈希操作
std::string hset_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string hget_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string hdel_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string hkeys_handler(std::vector<std::string> &args, RedisWrapper &engine);

// 链表操作
std::string lpush_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string rpush_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string lpop_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string rpop_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string llen_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string lrange_handler(std::vector<std::string> &args,
                           RedisWrapper &engine);

// 集合操作
std::string zadd_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string zrem_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string zrange_handler(std::vector<std::string> &args,
                           RedisWrapper &engine);
std::string zcard_handler(std::vector<std::string> &args, RedisWrapper &engine);
std::string zscore_handler(std::vector<std::string> &args,
                           RedisWrapper &engine);
std::string zincrby_handler(std::vector<std::string> &args,
                            RedisWrapper &engine);

std::string zrank_handler(std::vector<std::string> &args, RedisWrapper &engine);

// 无序集合操作
std::string sadd_handler(std::vector<std::string> &args, RedisWrapper &engine);

std::string srem_handler(std::vector<std::string> &args, RedisWrapper &engine);

std::string sismember_handler(std::vector<std::string> &args,
                              RedisWrapper &engine);

std::string scard_handler(std::vector<std::string> &args, RedisWrapper &engine);

std::string smembers_handler(std::vector<std::string> &args,
                             RedisWrapper &engine);