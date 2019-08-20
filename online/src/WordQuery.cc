#include "WordQuery.h"
#include "Redispool.h"

#include <json/json.h>

#include <fstream>
#include <sstream>
#include <iterator>
using std::ifstream;
using std::istringstream;

namespace wd
{

//根据余弦相似度对网页进行排序
struct SimilarityCompare
{
    SimilarityCompare(vector<double> base)
    : _base(base)
    {}
    
    bool operator()(const pair<int, vector<double>> & lhs, const pair<int, vector<double>> & rhs)
    {
        //都与基准向量进行计算
        double lhsCrossProduct = 0;
        double rhsCorssProduct = 0;
        double lhsVectorLength = 0;
        double rhsVectorLength = 0;

        for(size_t idx = 0; idx < _base.size(); ++idx)
        {
            lhsCrossProduct += (lhs.second)[idx] * _base[idx];
            rhsCorssProduct += (rhs.second)[idx] * _base[idx];
            lhsVectorLength += pow((lhs.second)[idx], 2);
            rhsVectorLength += pow((rhs.second)[idx], 2);
        }

        if(lhsCrossProduct / sqrt(lhsVectorLength) < rhsCorssProduct / sqrt(rhsVectorLength))
        {
            return false;
        }else{
            return true;
        }
    }

    vector<double> _base; //权重向量
};

WordQuery::WordQuery(Configuration & conf)
: _conf(conf)
{
    //加载网页库
    loadLibrary();
}

//加载网页库
void WordQuery::loadLibrary()
{
    //读取网页库和网页偏移库
    ifstream newPageLib(_conf.getConfigMap()["newpagelib"]);
    ifstream newOffset(_conf.getConfigMap()["newoffsetlib"]);
    if(!newPageLib.good() || !newOffset.good())
    {
        cout << ">> loadLibrary() ifstream open file error!!" << endl;
        return;
    }
    
    string line;
    while(getline(newOffset, line))
    {
        istringstream iss(line);
        int docId, offset, docLen;
        iss >> docId >> offset >> docLen;

        string doc;
        doc.resize(docLen, ' ');
        newPageLib.seekg(offset, newPageLib.beg);
        newPageLib.read(&*doc.begin(), docLen);
        
        WebPage page(doc, _conf, _jieba);
        _pageLib.insert(std::make_pair(docId, page));
        //_offsetLib[docId] = std::make_pair(offset, docLen);
        _offsetLib.insert(std::make_pair(docId, std::make_pair(offset, docLen)));
    }
    cout << "loadLibrary() _pageLib.size() = " << _pageLib.size() << endl;
    cout << "loadLibrary() _offsetLib = " << _offsetLib.size() << endl;
    newPageLib.close();
    newOffset.close();

    //读取倒排索引库
    ifstream invertIndex(_conf.getConfigMap()["invertindexlib"]);
    if(!invertIndex.good())
    {
        cout << "ifstream open invertindexlib error!" << endl;
    }

    while(getline(invertIndex, line))
    {
        istringstream iss(line);
        string word;
        int docId;
        double weigth;
        
        iss >> word;
        while(iss >> docId >> weigth)
        {
            _invertIndexTable[word].insert(std::make_pair(docId, weigth));
        }
    }

    invertIndex.close();
}

//执行查询
string WordQuery::doQuery(const string & str)
{
    //首先查询缓存
    Redis * connect = Redispool::createRedisPool()->pop();
    string result = connect->get(str);
    Redispool::createRedisPool()->push(connect);
    connect = nullptr;
    if(result != "")
    {
        return result;
    }

    vector<string> queryWords;
    queryWords = _jieba(str.c_str());

    //只要其中一个查询词不在索引表中，就认为没有找到相关的网页
    cout << "查询词分词结果: ";
    for(auto word : queryWords)
    {
        cout << word << "  ";
        auto it = _invertIndexTable.find(word);
        if(it == _invertIndexTable.end())
        {
            cout << " >> not find " << word << endl;
            return returnNoAnswer();
        }
    }
    cout << endl;

    //计算每个词的权重
    vector<double> weigthList = getQueryWordsWeightVector(queryWords);
    SimilarityCompare similarityCmp(weigthList);

    //执行查询
    vector<pair<int, vector<double>>> resultVec;
    if(executeQuery(queryWords, resultVec))
    {
        stable_sort(resultVec.begin(), resultVec.end(), similarityCmp);
        vector<int> docIdVec;

        //得到包含查询关键字的所有文档Id
        for(auto elem : resultVec)
        {
            docIdVec.push_back(elem.first);
        }
        string queryResult = createJson(docIdVec, queryWords);
        //添加到缓存
        Redis * connect2 = Redispool::createRedisPool()->pop();
        connect2->set(str, queryResult);
        Redispool::createRedisPool()->push(connect2);
        connect2 = nullptr;

        return queryResult;
    }else{
        return returnNoAnswer();
    }
}

string WordQuery::returnNoAnswer()
{
    Json::Value root;

    Json::Value elem;
    elem["title"] = "404, not found";
    elem["url"] = "";
    elem["summary"] = "I cannot find what you want. What a pity!";

    root["files"].append(elem);
    Json::StyledWriter writer;
    return writer.write(root);
}

//计算查询词的权重值
vector<double> WordQuery::getQueryWordsWeightVector(vector<string> & queryWords)
{
    //统计词频
    map<string, int> wordFreqMap;
    for(auto word : queryWords)
    {
        //TF
        ++wordFreqMap[word];
    }
    
    vector<double> weigths;
    int pageNum = _offsetLib.size();
    double weightSum = 0;
    for(auto word : queryWords)
    {
        int DF = _invertIndexTable[word].size();
        double IDF = log(static_cast<double>(pageNum) / (DF + 1)) / log(2);

        int TF = wordFreqMap[word];
        double w = IDF * TF;
        weightSum += pow(w, 2);
        weigths.push_back(w);
    }

    //归一化处理
    for(auto & elem : weigths)
    {
        elem = elem / sqrt(weightSum);
    }

    return weigths;
}

//执行查询
bool WordQuery::executeQuery(const vector<string> & queryWords, vector<pair<int, vector<double>>> & resultVec)
{
    if(queryWords.size() == 0)
    {
        cout << "empty string not found" << endl;
        return false;
    }
    
    unordered_map<string, set<int>> wordDocId;
    for(auto word : queryWords)
    {
        for(auto it : _invertIndexTable[word])
        {
            wordDocId[word].insert(it.first);
        }
    }

    set<int> docIds(wordDocId[queryWords[0]].begin(), wordDocId[queryWords[0]].end());
    for(size_t idx = 1; idx < queryWords.size(); ++idx)
    {
        set<int> temp;
        set_intersection(docIds.begin(), docIds.end(), wordDocId[queryWords[idx]].begin(), wordDocId[queryWords[idx]].end(), inserter(temp, temp.begin()));
        docIds.swap(temp);
        //for(auto id : docIds)
        //{
        //    int docid = wordDocId[queryWords[idx]].count(id);
        //    if(docid > 0)
        //    {
        //        temp.insert(id);
        //    }
        //}
        //docIds.swap(temp);
    }

    if(docIds.size() == 0)
    {
        cout << "没有包含所有关键词的文章" << endl;
        return false;
    }
    cout << "包含所有关键词的文章id集合：";
    for(auto id : docIds)
    {
        cout << id << " ";
    }
    cout << endl;

    //查找包含所有关键词的文章的单词的权重
    for(auto id : docIds)
    {
        pair<int, vector<double>> weigths;
        weigths.first = id;
        for(auto word : queryWords)
        {
            for(auto index : _invertIndexTable[word])
            {
                if(index.first == id)
                {
                    weigths.second.push_back(index.second);
                    break;
                }
            }
        }
        resultVec.push_back(weigths);
    }

    return true;
}

string WordQuery::createJson(vector<int> & docIdVec, const vector<string> & queryWords)
{
    Json::Value root;
    
    int cnt = 0;
    for(auto docId : docIdVec)
    {
        Json::Value elem;
        elem["title"] = _pageLib[docId].getTitle();
        elem["url"] = _pageLib[docId].getUrl();
        elem["summary"] = _pageLib[docId].summary(queryWords);

        root["files"].append(elem);
        if(++cnt == 100)  //最多记录一百条
        {
            break;
        }
    }

    Json::StyledWriter writer;
    return writer.write(root);
}


}//end of namespace wd
