#include "Redispool.h"

namespace wd
{

Redispool * Redispool::_redisPool = nullptr;

Redispool * Redispool::createRedisPool()
{
    if(_redisPool == nullptr)
    {
        _redisPool = new Redispool();
    }
    return _redisPool;
}

void Redispool::destory()
{
    if(_redisPool)
    {
        delete _redisPool;
    }
}

Redispool::Redispool()
: _queSize(5)
, _mutex()
, _notEmpty(_mutex)
{
    for(int i = 0; i < _queSize; ++i)
    {
        Redis * temp = new Redis();
        //cout << "connect 之上" << endl;
        if(!temp->connect("127.0.0.1", 6379))
        {
            printf("redis connect error\n");
        }
        _que.push(temp);
    }
    cout << "Redispool()" << endl;
}

Redispool::~Redispool()
{
    while(!empty())
    {
        Redis * temp = _que.front();
        delete temp;
        _que.pop();
    }
}

bool Redispool::empty() const
{
    return _que.size() == 0;
}

void Redispool::push(Redis * connect)
{
    {
        MutexLockGuard autoLock(_mutex);
        _que.push(connect);
    }
    _notEmpty.notify();
}

Redis * Redispool::pop()
{
    Redis * temp;
    {
        MutexLockGuard autoLock(_mutex);
        while(empty())
        {
            _notEmpty.wait(); 
        }
        temp = _que.front();
        _que.pop();
    }
    cout << "Redispool::pop()" << endl;
    return temp;
}

}
