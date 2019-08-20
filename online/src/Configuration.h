#pragma once
#include <string>
#include <set>
#include <map>
using std::string;
using std::map;
using std::set;

namespace wd
{

class Configuration
{
public:
    Configuration(const string & filepath);
    
    map<string, string> & getConfigMap();  //获取存放位置文件内容的map
    set<string> &  getStopWordLisp();  //获取停用词集合
private:
    void init();   //读取配置文件内容
private:
    string _filepath;  //配置文件路径
    map<string, string> _configMap;  //配置文件内容
    set<string> _stopWordList;  //停用词集合
};

}//end of namespace wd
