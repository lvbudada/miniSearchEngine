#include "TcpConnection.h"
#include "EventLoop.h"

#include <sstream>
using std::ostringstream;

namespace wd
{

TcpConnection::TcpConnection(int fd, EventLoop * loop)
: _peersock(fd)
, _socketIO(fd)
, _localAddr(getLocalAddr())
, _peerAddr(getPeerAddr())
, _isShutdownWrite(false)
, _loop(loop)
{

}

TcpConnection::~TcpConnection()
{
    if(!_isShutdownWrite)
    {
        shutdown();
    }
}

string TcpConnection::receive()
{
    char buff[65536] = {0};
    _socketIO.readline(buff, sizeof(buff));
    return string(buff);
}

void TcpConnection::send(const string & msg)
{
    _socketIO.writen(msg.c_str(), msg.size());
}

void TcpConnection::sendInLoop(const string & msg)
{
    //通过EventLoop的指针调用runInLoop，将send(msg)方法注册到_pendingFunctos
    _loop->runInLoop(std::bind(&TcpConnection::send, this, msg));
}

string TcpConnection::toString() const
{
    ostringstream oss;  //字符串拼接
    oss << _localAddr.ip() << ":" << _localAddr.port() << " ---> "
        << _peerAddr.ip() << ":" << _peerAddr.port();
    return oss.str();
}

void TcpConnection::shutdown()
{
    if(!_isShutdownWrite)
    {
        _isShutdownWrite = true;
        _peersock.shutdownWrite();
    }
}

InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int ret = getsockname(_peersock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getsockname");
    }
    return InetAddress(addr);
}

InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int ret = getpeername(_peersock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getpeername");
    }
    return InetAddress(addr);
}

void TcpConnection::setConnectionCallback(const TcpConnectionCallback & cb)
{
    _onConnection = std::move(cb);
}

void TcpConnection::setMessageCallback(const TcpConnectionCallback & cb)
{
    _onMessage = std::move(cb);
}

void TcpConnection::setCloseCallback(const TcpConnectionCallback & cb)
{
    _onClose = std::move(cb);
}

void TcpConnection::handleConnectionCallback()
{
    if(_onConnection)
    {
        _onConnection(shared_from_this());
    }
}

void TcpConnection::handleMessageCallback()
{
    if(_onMessage)
    {
        _onMessage(shared_from_this());
    }
}

void TcpConnection::handleCloseCallback()
{
    if(_onClose)
    {
        _onClose(shared_from_this());
    }
}

}//end of namespace wd

