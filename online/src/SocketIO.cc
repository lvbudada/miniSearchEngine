#include "SocketIO.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace wd
{

SocketIO::SocketIO(int fd)
: _fd(fd)
{

}

//从接受缓冲区读取数据
int SocketIO::readn(char * buff, int len)
{
    int left = len;  //left表示“剩下的”意思
    char * p = buff;
    while(left > 0)
    {
        int ret = read(_fd, p, left);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }else if (-1 == ret){
            perror("read");
            return len - left;
        }else if (0 == ret){ //zero indicates end of file
            return len - left;
        }else{
            left -= ret;
            p += ret;
        }
    }

    return len - left;
}

//每一次读取一行数据
int SocketIO::readline(char * buff, int maxlen)
{
    int left = maxlen - 1; //减一是为了让最后一位为\0
    char * p = buff;
    int ret;
    int total = 0;  
    while(left > 0)
    {
        ret = recvPeek(p, left);
        //查找\n
        for(int idx = 0; idx != ret; ++idx)
        {
            //找到\n
            if(p[idx] == '\n')
            {
                int sz = idx + 1;
                readn(p, sz);
                total += sz;
                p += idx;   //将受到数据的\n替换成\0
                *p = '\0';
                return total;
            }
        }
        //如果没有发现\n
        readn(p, ret);
        left -= ret;
        p += ret;
        total += ret;
    }
    *p = '\0'; //最终没有发现'\n'
    return total;
}

int SocketIO::recvPeek(char * buff, int len)
{
    int ret;
    do
    {
        ret = recv(_fd, buff, len, MSG_PEEK);
    }while(-1 == ret && errno == EINTR);

    return ret;
}

int SocketIO::writen(const char * buff, int len)
{
    int left = len;
    const char * p = buff;
    while(left > 0)
    {
        int ret = write(_fd, p, left);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }else if (-1 == ret)
        {
            perror("write");
            return len - left;
        }else{
            left -= ret;
            p += ret;
        }
    }
    return len -left;
}

}//end of namespace wd
