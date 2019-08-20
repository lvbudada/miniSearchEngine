#include "WordQueryServer.h"
#include "Redispool.h"

namespace wd
{

WordQueryServer::WordQueryServer(const string & filename)
: _conf(filename)
, _wordQuery(_conf)
, _tcpServer(_conf.getConfigMap()["ip"], stoi(_conf.getConfigMap()["port"]))
, _threadpool(stoi(_conf.getConfigMap()["threadNum"]), stoi(_conf.getConfigMap()["queSize"]))
{

}

//开始提供服务
void WordQueryServer::start()
{
    _threadpool.start();
    
    //加载缓存池
    Redispool::createRedisPool();

    _tcpServer.setConnectionCallback(std::bind(&WordQueryServer::onConnection, this, std::placeholders::_1));
    _tcpServer.setMessageCallback(std::bind(&WordQueryServer::onMessage, this, std::placeholders::_1));
    _tcpServer.setCloseCallback(std::bind(&WordQueryServer::onClose, this, std::placeholders::_1));

    _tcpServer.start(); //epoll会一直循环监听

}

void WordQueryServer::onConnection(const TcpConnectionPtr & conn)
{
    cout << conn->toString() << "has connected!" << endl;
}

void WordQueryServer::onMessage(const TcpConnectionPtr & conn)
{
    cout << "onMessage...." << endl;
    string msg = conn->receive();
    cout << ">> receive msg from clinet: " << msg << endl;

    //业务逻辑的处理交给线程池去处理
    //decode
    //compute
    //encode
    
    _threadpool.addTask(std::bind(&WordQueryServer::doTaskThread, this, conn, msg));
}

void WordQueryServer::onClose(const TcpConnectionPtr & conn)
{
    cout << "onClose..." << endl;
    cout << conn->toString() << "has closed!" << endl;
}

void WordQueryServer::doTaskThread(const TcpConnectionPtr & conn, const string & msg)
{
    //获取查询结果
    string jsonData = _wordQuery.doQuery(msg);

    int jsonLength = jsonData.size();
    string queryResult = std::to_string(jsonLength).append("\n").append(jsonData);

    //将send函数注册到IO线程
    conn->sendInLoop(queryResult);
}

}
