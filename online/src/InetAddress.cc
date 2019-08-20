#include "InetAddress.h"

#include <string.h>

namespace wd
{

InetAddress::InetAddress(unsigned short port)
{
    ::memset(&_addr, 0, sizeof(_addr));  //将结构体清空
    _addr.sin_family = AF_INET; //表示采用Ipv4地址协议， 小端模式   大端模式
    _addr.sin_port = htons(port); //主机字节学转成网络字节序
    _addr.sin_addr.s_addr = INADDR_ANY;  //系统自动填入本机IP地址，inet_addr("0.0.0.0")代表本机地址（一个服务器可能有多个网卡）
}

InetAddress::InetAddress(const string & ip, unsigned short port)
{
    ::memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    _addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const struct sockaddr_in & addr)
: _addr(addr)  //c++结构体可以直接赋值
{
    //memset(&_addr, 0, sizeof(_addr));
    //_addr.sin_family = addr.sin_family;
    //_addr.sin_port = addr.sin_port;
    //_addr.sin_addr.s_addr = addr.sin_addr.s_addr;
}

string InetAddress::ip() const
{
    return string(::inet_ntoa(_addr.sin_addr));
}

unsigned short InetAddress::port() const
{
    return ntohs(_addr.sin_port);  //网络字节序转为主机字节序
}

}//end of namespace wd

