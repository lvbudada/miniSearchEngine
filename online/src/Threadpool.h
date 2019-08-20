#pragma once
#include "TaskQueue.h"

#include <vector>
#include <memory>
using std::vector;
using std::unique_ptr;

namespace wd
{

class Thread;

class Threadpool
{
    friend class WorkerThread;
public:
    Threadpool(size_t threadNum = 4, size_t queSize = 10);
    
    void start();
    void stop();
    void addTask(Task && task);

    ~Threadpool();
private:
    Task getTask();
    void threadfunc();

private:
    size_t _threadNum;
    size_t _queSize;
    vector<unique_ptr<Thread>> _threads;
    TaskQueue _taskQue;
    bool _isExit;
};

} //end of namespace wd
