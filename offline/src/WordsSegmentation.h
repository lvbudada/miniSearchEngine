#pragma once
#include "cppjieba/Jieba.hpp"

#include <iostream>
#include <vector>
#include <string>
using std::vector;
using std::string;
using std::cout;
using std::endl;

const char* const DICT_PATH = "../data/dict/jieba.dict.utf8";
const char* const HMM_PATH = "../data/dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../data/dict/user.dict.utf8";
const char* const IDF_PATH = "../data/dict/idf.utf8";
const char* const STOP_WORD_PATH = "../data/dict/stop_words.utf8";

namespace wd
{

class WordsSegmentation
{
public:
    WordsSegmentation()
    : _jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH)
    {
        cout << "jieba init" << endl;
    }
    
    vector<string> operator()(const char * src)
    {
        vector<string> words;
        _jieba.CutAll(src, words);
        return words;
    }

private:
    cppjieba::Jieba _jieba;
};

}//end od namespace wd
