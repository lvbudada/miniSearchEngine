#include "PageLib.h"
#include "tinyxml2.h"
using namespace tinyxml2;

#include <iostream>
#include <regex>
#include <fstream>
using std::cout;
using std::endl;
using std::regex;
using std::ofstream;

namespace wd
{

PageLib::PageLib(Configuration & conf, DirScanner & dirScanner)
: _conf(conf)
, _dirScanner(dirScanner)
{}

void PageLib::create()
{
    vector<string> & files = _dirScanner.files();
    for(auto it = files.begin(); it != files.end(); ++it)
    {
        parse(*it);
    }
}

void PageLib::parse(const string & filepath)
{
    XMLDocument doc;
    doc.LoadFile(filepath.c_str());
    if(doc.ErrorID())
    {
        cout << "doc load " << filepath << " error!" << endl;
        return;
    }
    cout << filepath << endl;
    XMLElement * root = doc.FirstChildElement();
    XMLElement * channel = root->FirstChildElement("channel");
    if(channel)  //防止空指针
    {
        XMLElement * itemElement =  channel->FirstChildElement("item");
        size_t num = _ripepageLib.size() + 1;
        for(; itemElement; itemElement = itemElement->NextSiblingElement("item"))
        {
            //cout << itemElement->GetText() << endl;
            XMLElement * titleElement = itemElement->FirstChildElement("title");
            XMLElement * linkElement = itemElement->FirstChildElement("link");
            XMLElement * descriptionElement = itemElement->FirstChildElement("description");

            const char * title = titleElement->GetText();
            const char * link = linkElement->GetText();
            const char * description = descriptionElement->GetText();

            XMLElement * contentElement = itemElement->FirstChildElement("content:encoded");
            const char * content = nullptr;
            if(contentElement)
            {
                content = contentElement->GetText();
            }else{
                content = description;
            }

            regex re("<[^>]*>");
            string contentProcess = std::regex_replace(content, re, "");
            string article = "<doc>\n"
                "  <docid>" + std::to_string(num) + "</docid>\n"
                "  <title>" + (title ? title : "") + "</title>\n"
                "  <link>" + (link ? link : "") + "</link>\n"
                "  <content>" + contentProcess + "</content>\n"
                "</doc>\n";
            _ripepageLib.push_back(article);
            ++num;
        }
    }
    cout << "PageLib::parse" << endl;
}

//存储网页课库和网页偏移库
void PageLib::store()
{
    ofstream pageLib(_conf.getConfigMap()["ripepagelib"]);
    ofstream offsetLib(_conf.getConfigMap()["offsetlib"]);
    if(!pageLib.good() || !offsetLib.good())
    {
        cout << "PageLib::store ofstream open file error" << endl;
        return;
    }

    for(size_t idx = 0; idx < _ripepageLib.size(); ++idx)
    {
        offsetLib << idx + 1 << '\t' <<  pageLib.tellp() << '\t' << _ripepageLib[idx].size() << '\n';
        pageLib << _ripepageLib[idx];
    }

    pageLib.close();
    offsetLib.close();
}

}//end of namespace wd
