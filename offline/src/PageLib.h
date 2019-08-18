#pragma once
#include "Configuration.h"
#include "DirScanner.h"

#include <vector>
#include <string>
#include <map>
using std::vector;
using std::string;
using std::map;
using std::pair;

namespace wd
{

class PageLib
{
public:
    PageLib(Configuration & conf, DirScanner & dirScanner);
    
    void create();  //创建网页库
    void store();  //存储网页库和位置偏移库
private:
    void parse(const string & filepath); //解析rss文件
private:
    Configuration & _conf;  //配置文件对象的引用
    DirScanner & _dirScanner;
    //vector<string> _rssWeb;  //rss源地址集合
    vector<string> _ripepageLib;  //存放格式化之后网页的容器
    map<int, pair<int, int>> _offsetLib;  //存放每篇文章在网页库的配置信息
    //docid  offset  length
};

}//end of namespace wd
