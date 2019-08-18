#pragma once
#include "Configuration.h"
#include "WordsSegmentation.h"

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;
using std::set;

namespace wd
{
class WebPage
{
    //判断两篇文章是否相等
    friend bool operator==(const WebPage & lhs, const WebPage & rhs);
    //对文档按Docid进行排序
    friend bool operator<(const WebPage & lhs, const WebPage & rhs);
public:
    const static int TOPK_NUMBER = 20;
    WebPage(string & doc, Configuration & conf, WordsSegmentation & jieba);

    int getDocId();  //获取文档ID
    string getDoc();  //获取文档
    map<string, int> & getWordsMap();  //获取文档的词频统计的map
private:
    void processDoc(Configuration & conf, WordsSegmentation & jieba);  //对格式化文档进行处理
    void calcTopK(vector<string> & wordsVector, int k, set<string> & stopWordList);  //获取文档的topk词集

private:
    string _doc;  //整篇文档，包含xml在内
    int _docId;  //文档id
    string _docTitle;  //文档标题
    string _docUrl;  //文档url
    string _docContent;  //文档内容
    string _docSummmary;  //需自动生成，不是固定的
    vector<string> _topWords;  //词频最高的前20个词
    map<string, int> _wordsMap;  //保存每篇文档的所有词语和词频，不包含停用词
};

}//end of namespace wd
