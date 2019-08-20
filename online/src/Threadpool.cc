#include "Threadpool.h"
#include "Thread.h"

#include <unistd.h>

#include <iostream>

namespace wd
{

Threadpool::Threadpool(size_t threadNum, size_t queSize)
: _threadNum(threadNum)
, _queSize(queSize)
, _taskQue(queSize)
, _isExit(false)
{
    _threads.reserve(threadNum);
}

Threadpool::~Threadpool()
{
    if(!_isExit)
    {
        stop();
    }
}

void Threadpool::addTask(Task && task)
{
    _taskQue.push(task);    
}

Task Threadpool::getTask()
{
    return _taskQue.pop();
}

void Threadpool::start()
{
    for(size_t idx = 0; idx < _threadNum; ++idx)
    {
        //_threads.push_back(unique_ptr<Thread>(new WorkerThread(*this)));
        unique_ptr<Thread> thread(new Thread(std::bind(&Threadpool::threadfunc, this)));
        _threads.push_back(std::move(thread));
    }

    for(auto & thread : _threads)
    {
        thread->start();
    }
}

void Threadpool::stop()
{
    if(!_isExit)
    {
        while(!_taskQue.empty()) //当任务队列非空，循环等待线程取任务
        {
            ::sleep(1);  //主线程
        }

        _isExit = true;
        _taskQue.wakeup();
        for(auto & thread : _threads)
        {
            thread->join();
        }
    }
}

//每一个子线程要完成的任务，其运行在
//WorkerThread::run方法中
void Threadpool::threadfunc()
{
    while(!_isExit)
    {
        Task task = getTask();
        if(task)
        {
            task();//当任务执行速度过快
            //在还没有将_isExit设置为true之前，每一个子线程
            //已经阻塞在了getTask()方法之上
        }
    }
}

}//end of namespace wd
