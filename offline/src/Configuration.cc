#include "Configuration.h"

#include <fstream>
#include <iostream>
#include <sstream>
using std::ifstream;
using std::istringstream;
using std::cout;
using std::endl;

namespace wd
{

Configuration::Configuration(const string & filepath)
: _filepath(filepath)
{
    init(); 
}

void Configuration::init()
{
    ifstream ifs(_filepath);
    if(!ifs)
    {
        cout << ">> ifstream open " << _filepath << " error" << endl;
        return;
    }

    string line;
    while(getline(ifs, line))
    {
        string key, value;
        istringstream iss(line);
        iss >> key >> value;
        _configMap[key] = value;
    }

    ifs.close();
}

map<string, string> & Configuration::getConfigMap()
{
    return _configMap;
}

set<string> & Configuration::getStopWordLisp()
{
    if(_stopWordList.size() > 0)
    {
        return _stopWordList;
    }

    ifstream ifs(_configMap["stopwords"]);
    if(!ifs.good())
    {
        cout << "getStopWordLisp ifstream open file error!" << endl;
    }

    string word;
    while(getline(ifs, word))
    {
        _stopWordList.insert(word);
    }
    return _stopWordList;
}

}//end of namespace wd
