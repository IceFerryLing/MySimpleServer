#include <iostream>
#include <mutex>
using namespace std;

//区别在于饿汉式在类加载时就创建对象，而懒汉式是在第一次调用GetInst()时创建对象。

//单例模式的实现方式一：饿汉式
class Single1{
private:
    //构造函数私有化
    Single1(){}

    Single1(const Single1&) = delete;               //禁止拷贝构造
    Single1 &operator = (const Single1&) = delete;  //禁止赋值操作

    static Single1* single;
    //类加载时就创建对象
    static Single1* GetInst(){
        return single;
    }
};


//单例模式的实现方式二：局部静态变量（懒汉式）
class Single2{
private:
    //构造函数私有化
    Single2(){}

    Single2(const Single2&) = delete;               //禁止拷贝构造
    Single2 &operator = (const Single2&) = delete;  //禁止赋值操作

public:
    static Single2& GetInst(){
        static Single2 single;
        return single;
    }
};

//静态成员变量指针方式

//饿汉式
class Single1Hungry{
private:
    //构造函数私有化
    Single1Hungry(){}
    Single1Hungry(const Single1Hungry&) = delete;               //禁止拷贝构造
    Single1Hungry &operator = (const Single1Hungry&) = delete;

public:
    static Single1Hungry* GetInst(){
        if(single == nullptr){
            single = new Single1Hungry();
        }
        return single;
    }

private:
    static Single1Hungry* single;
};

//懒汉式
class Single2Lazy{
private:
    //构造函数私有化
    Single2Lazy(){}
    Single2Lazy(const Single2Lazy&) = delete;               //禁止拷贝构造
    Single2Lazy &operator = (const Single2Lazy&) = delete;
    
public:
    static Single2Lazy* GetInst(){
        if(single != nullptr){
            return single;
        }

        s_mutex.lock();
        if(single != nullptr){
            s_mutex.unlock();
            return single;
        }
        single = new Single2Lazy();
        s_mutex.unlock();
        return single;
    }

private:
    static Single2Lazy* single;
    static std::mutex s_mutex;
};

//智能指针方式
class SingleAuto{
private:
    //构造函数私有化
    SingleAuto(){}
    SingleAuto(const SingleAuto&) = delete;               //禁止拷贝构造
    SingleAuto &operator = (const SingleAuto&) = delete;

public:
    static std::shared_ptr<SingleAuto> GetInst()
    {
        if (single != nullptr)
        {
            return single;
        }
        s_mutex.lock();         //为什么要加锁？因为可能有多个线程同时调用GetInst()
        if (single != nullptr)  //为什么还要再判断一次？因为可能有多个线程同时调用GetInst()，第一个线程创建了对象，其他线程就不需要再创建了
        {
            s_mutex.unlock();
            return single;
        }
        single = std::shared_ptr<SingleAuto>(new SingleAuto);
        s_mutex.unlock();
        return single;
    }
private:
    static std::shared_ptr<SingleAuto> single;
    static std::mutex s_mutex;
};

//safe detetor
//防止外界delete
//声明辅助类
//该类定义仿函数调用SingleAutoSafe的析构函数
//不可以提前声明SafeDetetor，因为SafeDetetor的析构函数中要用到SingleAutoSafe
//所以要提前声明SingleAutoSafe辅助类
class SingleAutoSafe;
class safeDetetor{
public:
    void operator()(SingleAutoSafe* sf){
        cout << "this is safe detetor!" << endl;
        if(sf != nullptr){
            delete sf;
            sf = nullptr;
        }
    }
};

class SingleAutoSafe{
private:
    //构造函数私有化
    SingleAutoSafe(){}
    ~SingleAutoSafe(){
        cout << "SingleAutoSafe destructor called!" << endl;
    }

    SingleAutoSafe(const SingleAutoSafe&) = delete;               //禁止拷贝构造
    SingleAutoSafe &operator = (const SingleAutoSafe&) = delete;

    //定义友元类，通过友元类访问私有析构函数
    friend class safeDetetor;
public:
    static std::shared_ptr<SingleAutoSafe> GetInst(){
        if (single != nullptr){
            return single;
        }

        s_mutex.lock();
        if (single != nullptr){
            s_mutex.unlock();
            return single;
        }

        //额外指定删除器
        single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe, safeDetetor());
        
        //也可以指定删除器为lambda表达式
        /*
        single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe, 
            [](SingleAutoSafe* sf){
                cout << "this is lambda detetor!" << endl;
                if(sf != nullptr){
                    delete sf;
                    sf = nullptr;
                }
            });
        */
        //也可以指定删除函数
        //single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe, SafeDelFunc);
        s_mutex.unlock();
        return single;
    }

private:
    static std::shared_ptr<SingleAutoSafe> single;
    static std::mutex s_mutex;
};

//通用的单例模板类
//定义通用删除器模板类

template <typename T>
class Single_T{
protected:
    Single_T() = default;
    Single_T(const Single_T<T> &st) = delete;
    Single_T &operator = (const Single_T<T> &st) = delete;

    ~Single_T(){
        cout << "this is auto safe template destruct" << endl;
    }

protected:
    static std::shared_ptr<T> GetInst(){
        if(single != nullptr){
            return single;
        }

        s_mutex.lock();
        if(single != nullptr){
            s_mutex.unlock();
            return single;
        }

        single = std::shared_ptr<T>(new T, SafeDeletor_T<T>());
        s_mutex.unlock();
        return single;
    }

private:
    static std::shared_ptr<T> single;
    static std::mutex s_mutex;
};

template <typename T>
std::shared_ptr<T> Single_T<T>::single = nullptr;
template <typename T>
std::mutex Single_T<T>::s_mutex;

//通过继承方式实现网络单例
template <typename T>
class SafeDetetor_T{
public:
    void operator()(T* t){
        cout << "this is safe detetor template!" << endl;
        if(t != nullptr){
            delete t;
            t = nullptr;
        }
    }
};

class SingleNet : public Single_T<SingleNet>{
private:
    SingleNet() = default;
    SingleNet(const SingleNet&) = delete;
    SingleNet &operator = (const SingleNet&) = delete;
    ~SingleNet() = default;

    friend class SafeDetetor_T<SingleNet>;
    friend class Single_T<SingleNet>;
};