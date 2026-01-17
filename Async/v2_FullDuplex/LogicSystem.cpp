#include "LogicSystem.h"
using namespace std;

LogicSystem::LogicSystem():_b_stop(false){
    RegisterCallBacks();//注册回调函数
    //启动逻辑处理线程
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}