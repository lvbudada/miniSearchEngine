#pragma once
#include "MutexLock.h"
#include "Condition.h"
#include "Redis.h"

#include <queue>
using std::queue;

namespace wd
{

class Redispool
{
public:
    static Redispool * createRedisPool();
    static void destory();

    bool empty() const;
    //bool full() const;
    void push(Redis *  connect);
    Redis * pop();
private:
    Redispool();
    ~Redispool();

private:
    static Redispool * _redisPool;
    int _queSize;
    queue<Redis *> _que;
    MutexLock _mutex;
    Condition _notEmpty;
    //Condition _notFull;
};

}
