#include "LogicSystem.h"
using namespace std;

//构造函数
LogicSystem::LogicSystem():_b_stop(false){
    RegisterCallBacks();//注册回调函数
    //启动逻辑处理线程
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::DealMsg(){
    for(;;){
        std::unique_lock<std::mutex> unique_lk(_mutex);

        //判断队列为空则用条件变量等待
        while(_msg_que.empty() && !_b_stop){
            _consume.wait(unique_lk);
        }

        //判断如果为关闭状态，取出逻辑队列所有数据及时处理并退出循环
        if(_b_stop){
            while(!_msg_que.empty()){
                auto msg_node = _msg_que.front();
                cout << "recv msg id is" << msg_node->_recvnode->_msg_id << endl;
                //call_back_iter是回调函数映射表的迭代器
                auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);

                if (call_back_iter == _fun_callback.end()){
                    //未找到对应的回调函数，跳过处理
                    _msg_que.pop();
                    continue;
                }
                
                call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
                    std::string(msg_node->_recvnode->_msg, msg_node->_recvnode->_cur_len));
                _msg_que.pop();
            }
            break;
        }

        // 如果没有停服，并且队列中有数据
        auto msg_node = _msg_que.front();
        cout << "recv msg id is" << msg_node->_recvnode->_msg_id << endl;

        auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);
        if(call_back_iter == _fun_callback.end()){
            _msg_que.pop();
            continue;
        }
    }
}



// 向消息队列投递消息
void LogicSystem::RegisterCallBacks(){
    _fun_callback[MSG_HELLO_WORD] = std::bind(&LogicSystem::HelloWorldCallBack, this, placeholders::_1, placeholders::_2, placeholders::_3);
}   

//处理消息的线程函数
void LogicSystem::HelloWorldCallBack(shared_ptr<Session> session, const short& msg_id, const string& msg_data){
    Json::Reader reader;   //reader是JSON解析器对象
    Json::Value root;      //root是JSON对象，类似于字典或对象
    //parse函数将msg_data字符串解析为JSON对象root
    reader.parse(msg_data, root);
    std::cout << "receive msg id is" << root["id"].asInt() << "msg data is ]"
        << root["data"].asString() << std::endl;
    //构造返回的JSON数据
    root["data"] = "server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString();
    session->Send(return_str, root["id"].asInt());
}

void LogicSystem::PostMsgToQue(shared_ptr<LogicNode> msg){
    std::unique_lock<std::mutex> unique_lk(_mutex);
    _msg_que.push(msg);
    
    if(_msg_que.size() == 1){
        _consume.notify_one();
    }
}

