#include "TcpServer.h"

namespace wd
{

TcpServer::TcpServer(const string & ip, unsigned short port)
: _acceptor(ip, port)
, _loop(_acceptor)
{

}

void TcpServer::setConnectionCallback(TcpConnectionCallback && cb) //右值引用不能绑定到左值
{
    _loop.setConnectionCallback(std::move(cb));
}

void TcpServer::setMessageCallback(TcpConnectionCallback && cb)
{
    _loop.setMessageCallback(std::move(cb));
}

void TcpServer::setCloseCallback(TcpConnectionCallback && cb)
{
    _loop.setCloseCallback(std::move(cb));
}

void TcpServer::start()
{
    _acceptor.ready();
    _loop.loop();
}

}//end of namespace wd
