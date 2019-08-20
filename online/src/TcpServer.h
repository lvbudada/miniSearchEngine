#pragma once
#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventLoop.h"

namespace wd
{

using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(const TcpConnectionPtr &)>;

class TcpServer
{
public:
    TcpServer(const string & ip, unsigned short port);
    void start();

    void setConnectionCallback(TcpConnectionCallback && cb);
    void setMessageCallback(TcpConnectionCallback && cb);
    void setCloseCallback(TcpConnectionCallback && cb);

private:
    Acceptor _acceptor;
    EventLoop _loop;
};

}//end of namespace wd
