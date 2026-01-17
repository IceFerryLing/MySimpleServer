#pragma once 
#include <memory>
#include <mutex>
#include <AccCtrl.h>
#include <iostream>

using namespace std;
template <typename T>
class Singleton{
private:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton &operator = (const Singleton<T>& st) = delete;

    static std::shared_ptr<T> _instance;

public:
    ~Singleton(){
        cout << "this is singleton destruct" << endl;
    }

    static std::shared_ptr<T> GetInst(){
        static std::once_flag s_flag;
        std::call_once(s_flag, [&](){
            _instance = shared_ptr<T>(new T);
        });
        return _instance;
    }

    void PrintAddress(){
        std::cout << _instance.get() << std::endl;
    }
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;