#include "../../include/redis_wrapper/redis_wrapper.h"
#include "../include/handler.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/Buffer.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include <exception>
#include <functional>
#include <string>
#include <vector>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

class RedisServer
{
public:
    RedisServer(EventLoop *loop, const InetAddress &listenAddr)
        : server_(loop, listenAddr, "RedisServer"), redis_("example_db")
    {
        server_.setConnectionCallback(std::bind(&RedisServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&RedisServer::onMessage, this, _1, _2, _3));
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO << "Receive connection from: " << conn->peerAddress().toIpPort();
        }
        else
        {
            LOG_INFO << "Close connection from: " << conn->peerAddress().toIpPort();
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
    {
        std::string msg = buf->retrieveAllAsString();
        LOG_INFO << "Receive message from: " << conn->peerAddress().toIpPort() << ": " << msg << "\n";
        if (msg == "PING\r\n")
        {
            conn->send("+PONG\r\n");
        }

        // 解析并处理请求
        std::string response = handleRequest(msg);
        conn->send(response);
    }
    TcpServer server_;
    RedisWrapper redis_;

    std::string handleRequest(const std::string &request)
    {
        // if (request == "*1\r\n$7\r\nzCOMMAND\r\n")
        //     return "+OK\r\n";
        size_t pos = 0;
        if (request.empty())
        {
            return "-ERR Protocol error: expected '*'\r\n";
        }

        int numElements = 0;
        try
        {
            numElements = std::stoi(request.substr(pos + 1));
        }
        catch (const std::exception &)
        {
            return "-ERR Protocol error: invalid number of arguments\r\n";
        }

        pos = request.find('\r', pos) + 1;

        std::vector<std::string> args;

        for (int i = 0; i < numElements; i++)
        {
            if (pos >= request.size())
            {
                return "-ERR Protocol error: expected '$'\r\n";
            }

            int len = 0;
            std::string value_len;
            int next_n_pos;
            try
            {
                next_n_pos = request.find('$', pos);
                std::cout << "next_n_pos + 1: " << request.substr(next_n_pos + 1) << std::endl;
                len = std::stoi(request.substr(next_n_pos + 1));
            }
            catch (const std::exception &)
            {
                return "-ERR Protocol error: invalid bulk length\r\n";
            }
            pos = next_n_pos + 4;
            if (pos + len > request.size())
            {
                return "-ERR Protocol error: invalid bulk length, exceeds request "
                       "size\r\n";
            }
            args.push_back(request.substr(pos, len));
            next_n_pos = request.find('\n', pos);
            pos = next_n_pos + 1;
        }

        // 处理命令
        std::cout << "args[0]: " << args[0] << std::endl;
        switch (string2OPS(args[0]))
        {
        case OPS::GET:
        {
            return this->redis_.get(args);
            // return get_handler(args, this->redis_);
        }
        case OPS::SET:
        {
            return this->redis_.set(args);
            // return set_handler(args, this->redis_);
        }
        case OPS::EXPIRE:
        {
            return this->redis_.expire(args);
            // return expire_handler(args, this->redis_);
        }
        case OPS::TTL:
        {
            return this->redis_.ttl(args);
            // return ttl_handler(args, this->redis_);
        }
        case OPS::HSET:
        {
            return this->redis_.hset(args);
            // return hset_handler(args, this->redis_);
        }
        case OPS::HGET:
        {
            return this->redis_.hget(args);
            // return hget_handler(args, this->redis_);
        }
        case OPS::ZADD:
        {
            return this->redis_.zadd(args);
            // return zadd_handler(args, this->redis_);
        }
        case OPS::ZRANGE:
        {
            return this->redis_.zrange(args);
            // return zrange_handler(args, this->redis_);
        }
        case OPS::SADD:
        {
            return this->redis_.sadd(args);
            // return sadd_handler(args, this->redis_);
        }
        case OPS::SREM:
        {
            return this->redis_.srem(args);
            // return srem_handler(args, this->redis_);
        }
        case OPS::SISMEMBER:
        {
            return this->redis_.sismember(args);
            // return sismember_handler(args, this->redis_);
        }
        case OPS::PING:
        {
            return "+PONG\r\n";
        }
        default:
        {
            return "-ERR unknown command\r\n";
        }
        }
    }
};

int main()
{
    EventLoop loop;
    InetAddress listenAddr(6379);

    RedisServer server(&loop, listenAddr);

    server.start();
    loop.loop();

    return 0;
}
