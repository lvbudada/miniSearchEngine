#pragma once

#include "Noncopyable.h"

#include <pthread.h>
#include <functional>
#include <string>
using std::function;
using std::string;

namespace wd
{


class Thread
: Noncopyable
{
public:
    using ThreadCallback = function<void()>;

    Thread(ThreadCallback && cb)
    : _pthid(0)
    , _isRuning(false)
    , _cb(std::move(cb))
    {}

    void start();
    void join();

    ~Thread();
private:
    static void* threadfunc(void *arg);
private:
    pthread_t _pthid;
    bool _isRuning;
    ThreadCallback _cb;
};

}//end of namespace wd
