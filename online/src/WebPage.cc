#include "WebPage.h"

#include <queue>
#include <algorithm>
#include <sstream>
using std::priority_queue;
using std::pair;
using std::istringstream;

namespace wd
{

struct WordFreqCompare
{
    bool operator()(const pair<string, int> & lhs, const pair<string, int> & rhs)
    {
        if(lhs.second < rhs.second)
        {
            return true;
        }else if (lhs.second == rhs.second && lhs.first > rhs.first){
            //按照字典序排列，选取字典去小的
            return true;
        }else{
            return false;
        }
    }
};

WebPage::WebPage(string & doc, Configuration & conf, WordsSegmentation & jieba)
: _doc(doc)
{
    processDoc(conf, jieba);
}

//获取文档ID
int WebPage::getDocId()
{
    return _docId;
}

//获取文档
string WebPage::getDoc()
{
    return _doc;
}

string WebPage::getTitle()
{
    return _docTitle;
}

string WebPage::getUrl()
{
    return _docUrl;
}

//获取文档的词频统计的map
map<string, int> & WebPage::getWordsMap()
{
    return _wordsMap;
}

//对格式化文档进行处理
void WebPage::processDoc(Configuration & conf, WordsSegmentation & jieba)
{
    string docIdHead = "<docid>";
    string docIdTail = "</docid>";
    string titleHead = "<title>";
    string titleTail = "</title>";
    string linkHead = "<link>";
    string linkTail = "</link>";
    string contentHead = "<content>";
    string contentTail = "</content>";

    int bpos = _doc.find(docIdHead);
    int epos = _doc.find(docIdTail);
    string docId = _doc.substr(bpos + docIdHead.size(), epos - bpos - docIdHead.size());
    _docId = stoi(docId);

    bpos = _doc.find(titleHead);
    epos = _doc.find(titleTail);
    _docTitle = _doc.substr(bpos + titleHead.size(), epos - bpos - titleHead.size());

    bpos = _doc.find(linkHead);
    epos = _doc.find(linkTail);
    _docUrl = _doc.substr(bpos + linkHead.size(), epos - bpos - linkHead.size());

    bpos = _doc.find(contentHead);
    epos = _doc.find(contentTail);
    _docContent = _doc.substr(bpos + contentHead.size(), epos - bpos - contentHead.size());
    
    //对文章内容分词
    vector<string> contentWords = jieba(_docContent.c_str());
    //去掉停词，获取词频最高的20词, 保存每篇文档所有词语和词频（不包含停词）
    calcTopK(contentWords, TOPK_NUMBER,conf.getStopWordLisp());
}

void WebPage::calcTopK(vector<string> & wordsVector, int k, set<string> & stopWordList)
{
    for(auto it = wordsVector.begin(); it != wordsVector.end(); ++it)
    {
        auto stopIt = stopWordList.find(*it);
        if(stopIt == stopWordList.end())
        {
            ++_wordsMap[*it];
        }
    }

    //使用优先队列统计词频最高的20词
    priority_queue<pair<string, int>, vector<pair<string, int>>, WordFreqCompare>
        wordFreq(_wordsMap.begin(), _wordsMap.end());

    //选取词频最高的二十个词
    while(!wordFreq.empty())
    {
        string word = wordFreq.top().first;
        wordFreq.pop();
        //去掉\n和\r
        if(word.size() == 1 && (static_cast<unsigned int>(word[0]) == 10
                            || static_cast<unsigned int>(word[0]) == 13))
        {
            continue;
        }
        _topWords.push_back(word);
        if(_topWords.size() >= static_cast<size_t>(k))
        {
            break;
        }
    }
}

//判断两篇文档是否相等
bool operator==(const WebPage & lhs, const WebPage & rhs)
{
    int commonNum = 0;
    for(auto it = lhs._topWords.begin(); it != lhs._topWords.end(); ++it)
    {
        commonNum += std::count(rhs._topWords.begin(), rhs._topWords.end(), *it);
    }

    int lhsNum = lhs._topWords.size();
    int rhsNum = rhs._topWords.size();
    int totalNum = lhsNum < rhsNum ? lhsNum : rhsNum;

    if(static_cast<double>(commonNum) / totalNum > 0.75)
    {
        return true;
    }else{
        return false;
    }
    return true;
}

//对文档按Docid进行排序
bool operator<(const WebPage & lhs, const WebPage & rhs)
{
    return lhs._docId < rhs._docId;
}

//自动生成摘要
string WebPage::summary(const vector<string> & queryWords)
{
    vector<string> summaryVec;
    istringstream iss(_docContent);
    string line;
    while(iss >> line)
    {
        for(auto word : queryWords)
        {
            if(line.find(word) != string::npos)
            {
                summaryVec.push_back(line);
                break;
            }
        }

        if(summaryVec.size() >= 3)
        {
            break;
        }
    }

    string summary;
    for(auto & elem : summaryVec)
    {
        summary.append(elem).append("\n");
    }

    return summary;
}

}//end of namespace wd
