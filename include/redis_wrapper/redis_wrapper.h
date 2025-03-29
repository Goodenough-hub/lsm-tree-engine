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
    std::string expire(std::vector<std::string> &args);
    std::string ttl(std::vector<std::string> &args);
    std::string hset(std::vector<std::string> &args);
    std::string hget(std::vector<std::string> &args);
    std::string zadd(std::vector<std::string> &args);
    std::string zrange(std::vector<std::string> &args);

private:
    // 清理过期的哈希hash
    bool expire_hash_clean(const std::string &key,
                           std::shared_lock<std::shared_mutex> &rlock);

    // 清理过期的有序集合zset
    bool expire_zset_clean(const std::string &key,
                           std::shared_lock<std::shared_mutex> &rlock);

private:
    std::unique_ptr<LSMEngine> lsm;
    std::shared_mutex redis_mtx;
};