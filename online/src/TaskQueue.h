#pragma once
#include "MutexLock.h"
#include "Condition.h"

#include <queue>
#include <functional>
using std::queue;

namespace wd
{

using Task = std::function<void()>;
using ElemType = Task;

class TaskQueue
{
public:
    TaskQueue(size_t queSize = 10);

    void push(ElemType &);
    ElemType pop();
    bool empty() const;
    bool full() const;

    void wakeup();//唤醒所有线程
private:
    size_t _queSize;
    queue<ElemType> _que;
    MutexLock _mutex;
    Condition _notFull;
    Condition _notEmpty;
    bool _flag;
};

}//end of namespace wd
