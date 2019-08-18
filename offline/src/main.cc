#include "Configuration.h"
#include "DirScanner.h"
#include "PageLib.h"
#include "PageLibPreprocessor.h"

#include <iostream>
using std::cout;
using std::endl;

int main()
{
    wd::Configuration conf("../conf/my.conf");
    //for(auto elem : conf.getConfigMap())
    //{
    //    cout << elem.first << "--->" << elem.second << endl;
    //}

    //wd::DirScanner dir(conf);
    //for(auto elem : dir.files())
    //{
    //    cout << elem << endl;
    //}
    //cout << dir.files().size() << endl;
    //wd::PageLib pageLib(conf, dir);
    //pageLib.create();
    //pageLib.store();
    
    wd::PageLibPreprocessor preProcess(conf);
    preProcess.doProcess();
    return 0;
}

