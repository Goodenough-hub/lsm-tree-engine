#include "../include/handler.h"

#include <algorithm>
#include <cctype> // 新增此头文件
#include <string>
#include <vector>

// 将字符串转为小写
std::string tolower(const std::string &str)
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return lowerStr;
}

OPS string2OPS(const std::string &opStr)
{
    std::string lowerOpStr = tolower(opStr);
    if (lowerOpStr == "get")
    {
        return OPS::GET;
    }
    else if (lowerOpStr == "ping")
    {
        return OPS::PING;
    }
    else if (lowerOpStr == "set")
    {
        return OPS::SET;
    }
    else if (lowerOpStr == "expire")
    {
        return OPS::EXPIRE;
    }
    else if (lowerOpStr == "ttl")
    {
        return OPS::TTL;
    }
    else if (lowerOpStr == "hget")
    {
        return OPS::HGET;
    }
    else if (lowerOpStr == "hset")
    {
        return OPS::HSET;
    }
    else if (lowerOpStr == "zadd")
    {
        return OPS::ZADD;
    }
    else if (lowerOpStr == "zrange")
    {
        return OPS::ZRANGE;
    }
    else if (lowerOpStr == "sadd")
    {
        return OPS::SADD;
    }
    else if (lowerOpStr == "srem")
    {
        return OPS::SREM;
    }
    else if (lowerOpStr == "sismember")
    {
        return OPS::SISMEMBER;
    }
    else
    {
        return OPS::UNKNOWN;
    }
}

std::string set_hander(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() != 3)
    {
        return "-EER wrong number of arguments for 'set' command";
    }
    return engine.set(args);
}

std::string get_handle(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() != 3)
    {
        return "-EER wrong number of arguments for 'get' command";
    }
    return engine.get(args);
}

std::string expire_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() != 2)
    {
        return "-EER wrong number of arguments for 'expire' command";
    }
    return engine.expire(args);
}

std::string ttl_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() != 2)
    {
        return "-EER wrong number of arguments for 'ttl' command";
    }
    return engine.ttl(args);
}

std::string hset_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() != 4)
    {
        return "-EER wrong number of arguments for 'hset' command";
    }
    return engine.hset(args);
}

std::string hget_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() != 3)
    {
        return "-EER wrong number of arguments for 'hget' command";
    }
    return engine.hget(args);
}

std::string zadd_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() < 3)
    {
        return "-EER wrong number of arguments for 'zadd' command";
    }
    return engine.zadd(args);
}

std::string zrange_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() < 4)
    {
        return "-EER wrong number of arguments for 'zrange' command";
    }
    return engine.zrange(args);
}

std::string sadd_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() < 3)
    {
        return "-EER wrong number of arguments for 'sadd' command";
    }
    return engine.sadd(args);
}

std::string srem_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() < 3)
    {
        return "-EER wrong number of arguments for 'srem' command";
    }
    return engine.srem(args);
}

std::string sismember_handler(std::vector<std::string> args, RedisWrapper &engine)
{
    if (args.size() < 3)
    {
        return "-EER wrong number of arguments for 'sismember' command";
    }
    return engine.sismember(args);
}