#include "PageLibPreprocessor.h"

#include <fstream>
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::istringstream;

namespace wd
{

PageLibPreprocessor::PageLibPreprocessor(Configuration & conf)
: _conf(conf)
{

}

//执行预处理
void PageLibPreprocessor::doProcess()
{
    readInfoFromFile();

    time_t t1 = time(NULL);
    cutRedundantPages();
    buildInvertIndexTable();
    time_t t2 = time(NULL);
    cout << "cutRedundantPages & buildInvertIndexTable = " << t2 - t1 << endl;

    time_t t3 = time(NULL);
    storeOnDisk();
    cout << "storeOnDisk = " << t3 - t2 << endl;
}

//根据配置信息读取网页库和位置偏移库的内容
void PageLibPreprocessor::readInfoFromFile()
{
    ifstream ripePageLib(_conf.getConfigMap()["ripepagelib"]);
    ifstream offsetlib(_conf.getConfigMap()["offsetlib"]);
    if(!ripePageLib.good() || !offsetlib.good())
    {
        cout << "readInfoFromFile() ifstream open file error!!" << endl;
    }

    //读取网页偏移库
    string line;
    int docid, docOffset, docLen;
    while(getline(offsetlib, line))
    {
        istringstream iss(line);
        iss >> docid >> docOffset >> docLen;
        
        string doc;
        doc.resize(docLen, ' ');
        //ripePageLib.seekg(docOffset, std::ios::beg);
        ripePageLib.seekg(docOffset, ripePageLib.beg);
        //根据网页偏移信息读取网页库
        ripePageLib.read(&*doc.begin(), docLen);

        _offsetLib.insert(std::make_pair(docid, std::make_pair(docOffset, docLen)));
        WebPage page(doc, _conf, _jieba);
        _pageLib.push_back(page);
    }
}

//对溶于的网页进行去重
void PageLibPreprocessor::cutRedundantPages()
{
    for(size_t i = 0; i != _pageLib.size() - 1; ++i)
    {
        for(size_t j = i + 1; j != _pageLib.size(); ++j)
        {
            if(_pageLib[i] == _pageLib[j])
            {
                _pageLib[j] = _pageLib.back();
                _pageLib.pop_back();
                --j;
            }
        }
    }
}

//创建倒排索引
//void PageLibPreprocessor::buildInvertIndexTable()
//{
//    for(auto it = _pageLib.begin(); it != _pageLib.end(); ++it)
//    {
//        //将每篇文章的词语、文章id和词频插入倒排索引表
//        for(auto & words : it->getWordsMap())
//        {
//            _invertIndexTable[words.first].insert(std::make_pair(it->getDocId(), words.second));
//        }
//    }
//
//    //总文章数
//    int totalPage = _pageLib.size();
//    //包含该词语的文档数量
//    map<string, int> _wordPages;
//    for(auto it = _invertIndexTable.begin(); it != _invertIndexTable.end(); ++it)
//    {
//        _wordPages[it->first] = it->second.size();
//    }
//
//    //逆文档频率IDF
//    //TF*IDF
//    for(auto it = _invertIndexTable.begin(); it != _invertIndexTable.end(); ++it)
//    {
//        for(auto & article : it->second)
//        {
//            article.second = article.second * (log(static_cast<double>(totalPage) / (_wordPages[it->first] + 1)));
//        }
//    }
//}

//创建倒排索引表 TF-IDF算法
void PageLibPreprocessor::buildInvertIndexTable()
{
    for(auto & page : _pageLib)
    {
        map<string, int> & wordsMap = page.getWordsMap();
        for(auto & wordFreq : wordsMap)
        {
            _invertIndexTable[wordFreq.first].push_back(std::make_pair(page.getDocId(), wordFreq.second));
        }
    }

    //计算每篇文档中词的权重，并归一化
    map<int, double> weightSum; //保存每一篇文档所有词的权重平方和，int代表docid
    int totalPageNum = _pageLib.size();

    for(auto & item : _invertIndexTable)
    {
        int DF = item.second.size();
        //求关键词item.first的逆文档频率
        double IDF = log(static_cast<double>(totalPageNum) / (DF + 1)) / log(2);

        for(auto & sitem : item.second)
        {
            double weigth = sitem.second * IDF;

            weightSum[sitem.first] += pow(weigth, 2);
            sitem.second = weigth;
        }
    }

    //归一化处理
    for(auto & item : _invertIndexTable)
    {
        for(auto & sitem : item.second)
        {
            sitem.second = sitem.second / sqrt(weightSum[sitem.first]);
        }
    }
}

//处理后的网页库、位置偏移库、倒排索引表写入磁盘
void PageLibPreprocessor::storeOnDisk()
{
    //先写网页库和位置偏移库
    ofstream newPageLib(_conf.getConfigMap()["newpagelib"]);
    ofstream newoffsetlib(_conf.getConfigMap()["newoffsetlib"]);
    ofstream invertIndexLib(_conf.getConfigMap()["invertindexlib"]);
    if(!newPageLib || !newoffsetlib || !invertIndexLib)
    {
        cout << "storeOnDisk ofstream open file error!!" << endl;
        return;
    }
    
    //对文档按照docid排序
    std::sort(_pageLib.begin(), _pageLib.end());
    cout << "文档总数：" << _pageLib.size() << endl;
    
    for(auto it = _pageLib.begin(); it != _pageLib.end(); ++it)
    {
        int docId = it->getDocId();
        int offset = newPageLib.tellp();
        int docLen = it->getDoc().size();

        newPageLib << it->getDoc();
        newoffsetlib << docId << '\t' << offset << '\t' << docLen << '\n';
    }

    newPageLib.close();
    newoffsetlib.close();

    //存储倒排索引表
    for(auto & item : _invertIndexTable)
    {
        invertIndexLib << item.first << '\t';
        for(auto & sitem : item.second)
        {
            invertIndexLib << sitem.first << '\t'  << sitem.second << '\t';
        }
        invertIndexLib << endl;
    }

    invertIndexLib.close();
}

}//end of namespace wd
