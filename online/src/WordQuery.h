#pragma once
#include "Configuration.h"
#include "WebPage.h"
#include "WordsSegmentation.h"

#include <unordered_map>
#include <set>
using std::unordered_map;
using std::set;
using std::pair;

namespace wd
{

class WordQuery
{
public:
    WordQuery(Configuration & conf);

    string doQuery(const string & str);  //执行查询，返回结果
private:
    void loadLibrary(); //加载网页库
    //计算查询词的权重值
    vector<double> getQueryWordsWeightVector(vector<string> & queryWords);//计算查询词的权重值
    //执行查询
    bool executeQuery(const vector<string> & queryWords, vector<pair<int, vector<double>>> & resultVec);  //执行查询
    //将查询结果封装成json字符串
    string createJson(vector<int> & docIdVec, const vector<string> & queryWords);
    string returnNoAnswer();
private:
    Configuration & _conf;  //配置文件的引用
    WordsSegmentation _jieba; //Jieba分词库对象
    unordered_map<int, WebPage> _pageLib;  //网页库
    unordered_map<int, pair<int, int>> _offsetLib;  //偏移库
    //倒排索引库
    unordered_map<string, set<pair<int, double>>> _invertIndexTable;  //倒排索引库
};

}//end of namespace wd
