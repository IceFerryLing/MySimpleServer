#pragma once
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

//定义全双工回调函数类型
//返回值是void，参数包括：shared_ptr<Session>表示会话对象，const short& msg_id表示消息ID，const string& msg_data表示消息数据内容
//const引用可以避免拷贝，提高效率
typedef std::function<void(shared_ptr<Session>, const short& msg_id, const string& msg_data)> FullCallback;


class LogicSystem : public Singleton<LogicSystem>{

};
