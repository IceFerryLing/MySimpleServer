#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Server_demo.h"

using namespace std;
using boost::asio::ip::tcp;
class Server; // 前向声明

class Session:public enable_shared_from_this<Session>{
public:
    Session(boost::asio::io_context& ioc, Server* server):_socket(ioc), _server(server){
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }

    ~Session(){
        std::cout << "Session destruct delete this" << this << std::endl;
    }

    //Socket()作用是返回当前会话的socket引用，以便服务器能够接受连接。
    tcp::socket& Socket(){
        return _socket;
    }

    //Start()方法用于启动会话，开始异步读取数据。
    void Start();
    std::string& GetUuid();
private:
    //处理读取数据的回调函数
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred, shared_ptr<Session> _self_shared);
    //处理写入数据的回调函数
    void handle_write(const boost::system::error_code& error, shared_ptr<Session> _self_shared);

    tcp::socket _socket;
    enum{max_length = 1024};
    char _data[max_length];
    Server* _server;
    std::string _uuid;
};

