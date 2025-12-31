#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <queue>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "MsgNode.h"

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

    //GetUuid()方法返回会话的唯一标识符UUID。
    std::string& GetUuid();

    //Send()方法用于发送数据到客户端。
    void Send(char* msg, int length);

    //粘包测试
    void PrintRecvData(char* data, int length);

private:
    //处理读取数据的回调函数
    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred, shared_ptr<Session> _self_shared);
    //处理写入数据的回调函数
    void HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared);
    //Socket对象，表示与客户端的连接
    tcp::socket _socket;
    //用于存储接收数据的缓冲区
    enum{MAX_LENGTH = 1024};
    //接收数据缓冲区
    char _recv_buffer[MAX_LENGTH];
    //指向服务器对象的指针，用于管理会话
    Server* _server;
    //会话的唯一标识符UUID
    std::string _uuid;
    // 发送消息队列和互斥锁
    std::queue<std::shared_ptr<MsgNode>> _send_queue;
    std::mutex _send_lock;

    // 接收消息结构
    std::shared_ptr<MsgNode> _recv_msg_node;
    bool _b_head_parsed = false; // 是否已解析消息头
    // 消息头结构
    std::shared_ptr<MsgNode> _recv_head_node;
};

