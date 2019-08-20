#pragma once

namespace wd
{

//负责数据的接受和发送
class SocketIO
{
public:
    explicit
    SocketIO(int fd);

    int readn(char *buff, int len);
    int readline(char *buff, int maxlen);  //读取一行数据
    int writen(const char * buff, int len);
private:
    int recvPeek(char * buff, int len); //MSG_PEEK查看数据，但不从系统缓冲区移走数据

private:
    int _fd;
};

}//end of namespace wd
