#pragma once

#include "../engine/engine.h"
#include <memory>
#include <shared_mutex>

class RedisWrapper
{
public:
    RedisWrapper(const std::string &path);

    std::string set(std::vector<std::string> &args);
    std::string get(std::vector<std::string> &args);
    std::string incr(std::vector<std::string> &args);
    std::string decr(std::vector<std::string> &args);
    std::string del(std::vector<std::string> &args);
    std::string expire(std::vector<std::string> &args);
    std::string ttl(std::vector<std::string> &args);
    std::string hset(std::vector<std::string> &args);
    std::string hget(std::vector<std::string> &args);
    std::string hdel(std::vector<std::string> &args);
    std::string hkeys(std::vector<std::string> &args);
    std::string zadd(std::vector<std::string> &args);
    std::string zrange(std::vector<std::string> &args);
    std::string sadd(std::vector<std::string> &args);
    std::string srem(std::vector<std::string> &args);
    std::string sismember(std::vector<std::string> &args);
    std::string lpush(std::vector<std::string> &args);
    std::string rpush(std::vector<std::string> &args);
    std::string lpop(std::vector<std::string> &args);
    std::string rpop(std::vector<std::string> &args);
    std::string lrange(std::vector<std::string> &args);
    std::string llen(std::vector<std::string> &args);

private:
    // 清理过期的哈希hash
    bool expire_hash_clean(const std::string &key,
                           std::shared_lock<std::shared_mutex> &rlock);

    // 清理过期的有序集合zset
    bool expire_zset_clean(const std::string &key,
                           std::shared_lock<std::shared_mutex> &rlock);

    // 清理过期的无序集合set
    bool expire_set_clean(const std::string &key,
                          std::shared_lock<std::shared_mutex> &rlock);

    bool expire_list_clean(const std::string &key,
                           std::shared_lock<std::shared_mutex> &rlock);

private:
    std::unique_ptr<LSMEngine> lsm;
    std::shared_mutex redis_mtx;
};