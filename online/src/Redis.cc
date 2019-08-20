#include "Redis.h"

namespace wd
{

Redis::Redis()
{
    printf("Redis()\n");
}

bool Redis::connect(const string & ip, int port)
{
    _connect = redisConnect(ip.c_str(), port);
    if(_connect == NULL || _connect->err)
    {
        printf("redisConnect Error: %s\n", _connect->errstr);
        return false;
    }
    return true;
}

string Redis::get(const string & key)
{
    string str = "";
    _replay = (redisReply*)redisCommand(_connect, "GET %s", key.c_str());
    if(_replay->type == REDIS_REPLY_NIL)
    {
        printf("redis中没有数据: %s\n", key.c_str());
        return str;
    }
    str = _replay->str;
    freeReplyObject(_replay);

    printf("从redis中取得数据: %s\n", key.c_str());
    return str;
}

void Redis::set(const string & key, const string & value)
{
    _replay = (redisReply*)redisCommand(_connect, "SET %s %s", key.c_str(), value.c_str());
    if(_replay->type == REDIS_REPLY_STATUS)
    {
        printf("%s\n", _replay->str);
    }
    freeReplyObject(_replay);
    printf("将数据添加到redis中: %s\n", key.c_str());
}

Redis::~Redis()
{
    if(_connect)
    {
        redisFree(_connect);
    }
    _connect = nullptr;
    _replay = nullptr;
}

}//end of namespace wd
