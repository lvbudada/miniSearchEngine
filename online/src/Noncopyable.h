#pragma once

namespace wd
{

class Noncopyable
{
protected:
    Noncopyable() {}
    ~Noncopyable() {}

    Noncopyable(const Noncopyable &) = delete; //=delete 禁用某个函数
    Noncopyable& operator=(const Noncopyable &) = delete;
};

}//end of namespace wd
