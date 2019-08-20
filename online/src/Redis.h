#pragma once

#include <hiredis/hiredis.h>
#include <string.h>

#include <string>
using std::string;

namespace wd
{

class Redis
{
public:
    Redis();
    ~Redis();

    bool connect(const string & ip, int port);
    void set(const string & key, const string & value);
    string get(const string & key);
private:
    redisContext * _connect;
    redisReply * _replay;
};

}//end of namespace wd
