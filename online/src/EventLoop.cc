#include "EventLoop.h"
#include "Acceptor.h"
#include "TcpConnection.h"

#include <unistd.h>
#include <sys/eventfd.h>

#include <iostream>
#include <assert.h>
using std::cout;
using std::endl;

namespace wd
{

EventLoop::EventLoop(Acceptor & acceptor)
: _efd(createEpollFd())
, _eventfd(createEventFd())
, _acceptor(acceptor)
, _eventList(1024)
, _isLooping(false)
{
    addEpollFdRead(acceptor.fd());
    addEpollFdRead(_eventfd);
}

void EventLoop::loop()
{
    _isLooping = true;
    while(_isLooping) //一直循环监听
    {
        waitEpollFd();
    }
}

void EventLoop::unloop()
{
    _isLooping = false;
}

void EventLoop::runInLoop(Functor && cb)
{
    {
        MutexLockGuard autolock(_mutex);
        _pendingFuctors.push_back(std::move(cb));
    }

    wakeup(); //通知机制
}

void EventLoop::waitEpollFd()
{
    int nready;
    do
    {
        nready = epoll_wait(_efd, &*_eventList.begin(), _eventList.size(), 5000);//epoll_wait超时时间设为5秒
    }while(-1 == nready && errno == EINTR);

    if(-1 == nready)
    {
        perror("epoll_wait");
        return;
    }else if (0 == nready)
    {
        cout << ">> epoll_wait timeout!" << endl;
    }else{
        if(nready == (int)_eventList.size())
        {
            _eventList.resize(2 * nready);
        }

        for(int idx = 0; idx != nready; ++idx)
        {
            int fd = _eventList[idx].data.fd;
            if(fd == _acceptor.fd())
            {
                //处理新连接
                if(_eventList[idx].events & EPOLLIN)
                {
                    handleNewConnection();
                }
            }else if(fd == _eventfd){
                //由IO线程负责向客户端发送数据
                if(_eventList[idx].events & EPOLLIN)
                {
                    handleRead();
                    cout << ">> before dePendingFunctors()" << endl;
                    doPendingFunctors(); //在这里发送数据
                    cout << ">> after doPendingFunctors()" << endl;
                }
            }else{
                //处理消息
                if(_eventList[idx].events & EPOLLIN)
                {
                    handleMessage(fd);
                }
            }
        }
    }
}

void EventLoop::handleNewConnection()
{
    int peerfd = _acceptor.accept();
    addEpollFdRead(peerfd);

    TcpConnectionPtr conn(new TcpConnection(peerfd, this)); //传递给TcpConnection EventLoop指针
    conn->setConnectionCallback(_onConnection);
    conn->setMessageCallback(_onMessage);
    conn->setCloseCallback(_onClose);

    _conns.insert(std::make_pair(peerfd, conn));

    conn->handleConnectionCallback();
}

void EventLoop::handleMessage(int fd)
{
    bool isClosed = isConnectionClosed(fd);
    auto iter = _conns.find(fd);
    assert(iter != _conns.end());  //运行时断言
    //void assert(int expression)，先计算表达式expression，如果其值为假（即为0），
    //先向stderr打印一条出错信息，然后abort终止程序运行

    if(!isClosed)
    {
        iter->second->handleMessageCallback();
    }else{
        delEpollFdRead(fd);
        iter->second->handleCloseCallback();
        _conns.erase(iter);
    }
}

void EventLoop::handleRead()
{
    uint64_t howmany;
    int ret = ::read(_eventfd, &howmany, sizeof(howmany));
    if(ret != sizeof(howmany))
    {
        perror("read");
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    int ret = ::write(_eventfd, &one, sizeof(one));
    if(ret != sizeof(one))
    {
        perror("write");
    }
}

void EventLoop::doPendingFunctors()
{
    vector<Functor> tmp;
    {
        MutexLockGuard autolock(_mutex);
        tmp.swap(_pendingFuctors);
    }

    for(auto & functor : tmp)
    {
        functor(); //执行回调函数
    }
}

bool EventLoop::isConnectionClosed(int fd)
{
    int ret;
    do{
        char buff[1024];
        ret = recv(fd, buff, sizeof(buff), MSG_PEEK);
    }while(-1 == ret && errno == EINTR);

    return ret == 0;
}

int EventLoop::createEpollFd()
{
    int efd = epoll_create1(0);
    if(-1 == efd)
    {
        perror("epoll_create1");
    }

    return efd;
}

int EventLoop::createEventFd()
{
    int fd = ::eventfd(0, 0);
    if(fd == -1)
    {
        perror("eventfd");
    }
    return fd;
}

void EventLoop::addEpollFdRead(int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;

    int ret = epoll_ctl(_efd, EPOLL_CTL_ADD, fd, &event);
    if(-1 == ret)
    {
        perror("epoll_ctl");
    }
}

void EventLoop::delEpollFdRead(int fd)
{
    struct epoll_event event;
    event.data.fd = fd;

    int ret = epoll_ctl(_efd, EPOLL_CTL_DEL, fd, &event);
    if(-1 == ret)
    {
        perror("epoll_ctl");
    }
}

}//end of namespace wd
