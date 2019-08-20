#pragma once

#include "MutexLock.h"

#include <sys/epoll.h>

#include <vector>
#include <map>
#include <memory>
#include <functional>
using std::vector;
using std::map;
using std::shared_ptr;
using std::function;

namespace wd
{

class Acceptor;
class TcpConnection;

//使用epoll
class EventLoop
{
public:
    using TcpConnectionPtr = shared_ptr<TcpConnection>; //TcpConnection不能赋值，所以使用智能指针管理
    using TcpConnectionCallback = function<void(const TcpConnectionPtr&)>;
    using Functor = std::function<void()>;

    EventLoop(Acceptor & acceptor);
    void loop();
    void unloop();
    void runInLoop(Functor && cb); //TcpConnection的sendInLoopd调用，注册回调函数到_pendingFunctors

    void setConnectionCallback(TcpConnectionCallback && cb)
    {  _onConnection = std::move(cb); }
    void setMessageCallback(TcpConnectionCallback && cb)
    {  _onMessage = std::move(cb); }
    void setCloseCallback(TcpConnectionCallback cb)
    {  _onClose = std::move(cb); }

private:
    void waitEpollFd();
    void handleNewConnection();
    void handleMessage(int fd);
    void handleRead(); //write处理eventfd可读
    void wakeup(); //read处理eventfd
    int createEpollFd();
    int createEventFd();
    void doPendingFunctors(); //执行_pendingFunctors中的回调函数

    void addEpollFdRead(int fd);
    void delEpollFdRead(int fd);
    bool isConnectionClosed(int fd);

private:
    int _efd;
    int _eventfd;
    Acceptor & _acceptor;
    vector<struct epoll_event> _eventList;
    map<int, TcpConnectionPtr> _conns;
    bool _isLooping;

    MutexLock _mutex;
    vector<Functor> _pendingFuctors;  //多个子线程都要注册的回调函数（TcpConnection的send函数），
    //临界资源，加锁保护

    TcpConnectionCallback _onConnection;  //回调函数（时间处理器），连接建立时处理的事情
    TcpConnectionCallback _onMessage; //对客户端数据的处理，消息到达时处理的事情
    TcpConnectionCallback _onClose;
};

}//end of namespace wd
