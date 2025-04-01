#pragma once
#include "../../include/redis_wrapper/redis_wrapper.h"
#include <cstring>

enum class OPS
{
    PING,
    SET,
    GET,
    EXPIRE,
    TTL,
    HGET,
    HSET,
    ZADD,
    ZRANGE,
    SADD,
    SREM,
    SISMEMBER,
    UNKNOWN
};

OPS string2OPS(const std::string &opStr);

std::string set_hander(std::vector<std::string> args, RedisWrapper &engine);
std::string get_handle(std::vector<std::string> args, RedisWrapper &engine);
std::string expire_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string ttl_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string hget_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string hset_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string zadd_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string zrange_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string sadd_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string srem_handler(std::vector<std::string> args, RedisWrapper &engine);
std::string sismember_handler(std::vector<std::string> args, RedisWrapper &engine);