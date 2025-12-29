#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;
class Server; // 前向声明

class Session{
public:
    Session(boost::asio::io_context& ioc, Server* server):_socket(ioc), _server(server){}

    //Socket()作用是返回当前会话的socket引用，以便服务器能够接受连接。
    tcp::socket& Socket(){
        return _socket;
    }

    //Start()方法用于启动会话，开始异步读取数据。
    void Start();
    
private:
    //处理读取数据的回调函数
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    //处理写入数据的回调函数
    void handle_write(const boost::system::error_code& error);

    tcp::socket _socket;
    enum{max_length = 1024};
    char _data[max_length];
    Server* _server;
};

