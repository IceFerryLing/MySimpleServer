#include "Session_demo.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
using namespace std;

class Server{
public:
    //构造函数，初始化io_context和acceptor，并开始接受连接
    Server(boost::asio::io_context& ioc, short  port);
    void ClearSession(std::string uuid);
private:
    //开始接受连接
    void StartAccept();
    //处理接受连接的回调函数
    void HandleAccept(shared_ptr<Session> new_session, const boost::system::error_code& error);
    //存储活动会话的映射区别是
    boost::asio::io_context& _ioc;
    //acceptor用于监听传入连接
    tcp::acceptor _acceptor;

    std::map<std::string, shared_ptr<Session>> _sessions;
};