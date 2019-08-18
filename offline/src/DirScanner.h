#pragma once
#include "Configuration.h"

#include <vector>
#include <string>
using std::vector;
using std::string;

namespace wd
{

class DirScanner
{
public:
    DirScanner(Configuration & conf);

    vector<string> & files();  //返回_vecFilesfiles的引用

private:
    void traverse(const string & dirPath);  //获取某一目录下的所有文件
private:
    Configuration & _conf;
    vector<string> _vecFilesfiles;  //存放每个语料文件的绝对路径
};

}//end of namespace wd
