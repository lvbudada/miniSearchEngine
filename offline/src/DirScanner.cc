#include "DirScanner.h"

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

namespace wd
{

DirScanner::DirScanner(Configuration & conf)
: _conf(conf)
{
    traverse(_conf.getConfigMap()["yuliao"]);
}

vector<string> & DirScanner::files()
{
    return _vecFilesfiles;
}

void DirScanner::traverse(const string & dirPath)
{
    DIR * dir = opendir(dirPath.c_str());
    if(!dir)
    {
        printf("dirname = %s\n", dirPath.c_str());
        perror("opendir");
    }

    //进入指定的目录
    //::chdir(dirPath.c_str());
    struct stat mystat;
    struct dirent * p;
    while((p = readdir(dir)) != NULL)
    {
        //获取目录中项的属性
        string filePath = dirPath + "/" + p->d_name;
        ::stat(filePath.c_str(), &mystat);
        //判断该项是不是目录
        if(S_ISDIR(mystat.st_mode))
        {  //该项是目录
            //printf("%s\n", p->d_name);
            if(!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            {
                continue;
            }else{
                string dPath = dirPath + "/" + p->d_name;
                traverse(dPath);
            }
        }else{
            //该向不是目录
            _vecFilesfiles.push_back(dirPath + "/" + p->d_name);
        }
    }
    closedir(dir);
}

}//end of namespace wd
