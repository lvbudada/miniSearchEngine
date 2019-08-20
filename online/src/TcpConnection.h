#pragma once

#include "Noncopyable.h"
#include "Socket.h"
#include "InetAddress.h"
#include "SocketIO.h"

#include <string>
#include <memory>
#include <functional>
using std::string;
using std::shared_ptr;
using std::function;

namespace wd
{

class TcpConnection;
using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(const TcpConnectionPtr&)>;

class EventLoop;

//表示一个连接
class TcpConnection
: Noncopyable
, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(int fd, EventLoop * );
    ~TcpConnection();

    string receive();  //接受一行数据，接受的数据中包含\n
    //string recen(int n);  //接受n字节数据
    void send(const string & msg);
    void sendInLoop(const string & msg);

    string toString() const;
    void shutdown();

    void setConnectionCallback(const TcpConnectionCallback & cb);
    void setMessageCallback(const TcpConnectionCallback & cb);
    void setCloseCallback(const TcpConnectionCallback & cb);

    void handleConnectionCallback();
    void handleMessageCallback();
    void handleCloseCallback();

private:
    InetAddress getLocalAddr();
    InetAddress getPeerAddr();

private:
    Socket _peersock;
    SocketIO _socketIO;
    InetAddress _localAddr;
    InetAddress _peerAddr;
    bool _isShutdownWrite;
    EventLoop * _loop;

    TcpConnectionCallback _onConnection;
    TcpConnectionCallback _onMessage;
    TcpConnectionCallback _onClose;
};

}//end of namespace wd
