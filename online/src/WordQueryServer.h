#pragma once
#include "Condition.h"
#include "WordQuery.h"
#include "TcpServer.h"
#include "Threadpool.h"

namespace wd
{

class WordQueryServer
{
public:
    WordQueryServer(const string & filename);
    
    void start();
private:
    //被注册回调函数，提供给TcpServer使用
    void onConnection(const TcpConnectionPtr & conn);
    void onMessage(const TcpConnectionPtr & conn);
    void onClose(const TcpConnectionPtr & conn);

    //该方法由线程池的某一个线程执行
    void doTaskThread(const TcpConnectionPtr & conn, const string & msg);
private:
    Configuration _conf;  //配置类对象
    WordQuery _wordQuery;  //查讯类对象
    TcpServer _tcpServer;  //TCP通信对象
    Threadpool _threadpool;  //执行查询的缓存线程池类
};

}//end of namespace wd
