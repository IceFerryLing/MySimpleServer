#pragma once
/*
====================================================
LogicSystem 设计与主要成员函数调用关系说明

1. RegisterCallBacks
    - 注册所有消息类型对应的处理函数（回调），初始化时调用。
    - 作用：将每种消息ID和对应处理函数（如 HelloWorldCallBack）放入 _fun_callback 映射表。

2. PostMsgToQue
    - 网络层/Session 收到新消息时调用，将消息投递到 _msg_que 队列。
    - 作用：生产者，将待处理消息放入队列，供逻辑线程消费。

3. DealMsg
    - 逻辑线程主循环，不断从 _msg_que 队列取消息。
    - 查找 _fun_callback 映射表，分发给对应回调（如 HelloWorldCallBack）处理。
    - 作用：消费者，负责消息分发和业务处理。

4. HelloWorldCallBack
    - 某种消息的具体处理函数。
    - 由 DealMsg 间接调用，完成实际业务逻辑。

调用链简述：
网络层收到消息 → PostMsgToQue → 消息入队 → DealMsg 线程取出 → 查找回调 → 调用 HelloWorldCallBack。
====================================================
*/
#include "Singleton.h"
#include <queue>
#include <thread>
#include "Session_demo.h"
#include <map>
#include <functional>
#include "const.h"
#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"
#include "LogicNode.h"

using namespace std;
//定义全双工回调函数类型
//返回值是void，参数包括：shared_ptr<Session>表示会话对象，const short& msg_id表示消息ID，const string& msg_data表示消息数据内容
//const引用可以避免拷贝，提高效率
typedef function<void(shared_ptr<Session>, const short& msg_id, const string& msg_data)> FullCallback;


class LogicSystem : public Singleton<LogicSystem>{
    friend class Singleton<LogicSystem>;

private:
    //私有化构造函数，禁止外部创建实例
    LogicSystem() = default;
    void RegisterCallBacks();
    void HelloWorldCallBack(shared_ptr<Session>, const short& msg_id, const string& msg_data);
    void DealMsg();

    std::queue<shared_ptr<LogicNode>> _msg_que;
    std::mutex mutex;
    std::condition_variable _consume;
    std::thread _worker_thread;
    bool _b_stop;
    //消息ID到回调函数的映射表
    std::map<short, FullCallback> _fun_callback;
public:
    //析构公有，因为单例类会被外部调用销毁
    ~LogicSystem() = default;

    void PostMsgToQue(shared_ptr<Session> session, shared_ptr<RecvNode> recvNode);
};

// ===================== 问题 =====================
// Q: 消息里面有相关的函数，然后找到函数的位置然后再实现函数作用，这样理解对吗？
// A: 是的。每个消息都有一个消息ID（msg_id），LogicSystem 用 map<msg_id, 回调函数> 关联消息类型和处理函数。
// DealMsg 线程取出消息后，根据消息ID查找对应的回调函数并调用，实现具体的业务逻辑。

// Q: 消息里面同时有函数的参数，这些参数是怎么储存和传递的？
// A: 消息参数（如会话对象、消息ID、消息内容）通常被封装在消息节点（如 LogicNode 或 RecvNode）里，
// 放进队列 _msg_que。DealMsg 线程取出消息节点后，将这些参数直接传递给回调函数。

// Q: 为什么不用 std::bind？
// A: 回调类型 FullCallback 已约定参数格式（会话对象、消息ID、消息内容），
// 每次调用都能直接传递最新参数，不需要提前绑定。这样更灵活高效，且每条消息都能带自己的参数。


