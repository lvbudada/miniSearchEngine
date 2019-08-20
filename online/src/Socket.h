#pragma once

namespace wd
{

//负责套接字的创建和关闭（RAII技术）
class Socket
{
public:
    Socket();  //调用socket函数创建
    explicit
    Socket(int fd);

    int fd() const;

    void shutdownWrite(); //主动断开连接，关闭写端

    ~Socket();

private:
    int _fd;
};

}//end if namespace wd
