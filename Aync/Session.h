#pragma once
#include <memory>
#include <queue>
#include "boost/asio.hpp"
using namespace boost;
using namespace std;
const int RECVSIZE = 1024;

// MsgNode: 消息节点类
// 设计原理：
// 1. 管理发送数据的生命周期。在异步发送中，底层缓冲区必须在发送完成前保持有效。
//    通过 shared_ptr<MsgNode> 传递给回调函数，确保数据在发送期间不被销毁。
// 2. 记录发送状态。_cur_len 记录已发送长度，_total_len 记录总长度，用于处理 TCP 拆包/半包情况下的部分发送。
class MsgNode{
public:
    // 构造函数：深拷贝数据到内部缓冲区 _msg
    MsgNode(const char* msg, int total_len):_total_len(total_len), _cur_len(0){
        _msg = new char[total_len];
        memcpy(_msg, msg, total_len);
    }

    MsgNode(int total_len):_total_len(total_len), _cur_len(0){
        _msg = new char[total_len];
    }

    ~MsgNode(){
        delete[] _msg;
    }
    friend class Session;

private:
    int _total_len; // 消息总长度
    int _cur_len;   // 当前已发送长度
    char* _msg;     // 消息数据缓冲区
};

// Session: 会话类
// 设计原理：
// 1. 封装 TCP socket，管理连接的生命周期。
// 2. 提供异步读写接口。
class Session{
public:
    Session(std::shared_ptr<asio::ip::tcp::socket> socket);
    void Connect(const asio::ip::tcp::endpoint& endpoint);
    void WriteCallBackErr(const boost::system::error_code& error
                            , size_t bytes_transferred
                            , std::shared_ptr<MsgNode> msg);
    void WriteToSocketErr(const std::string& buf);

    void WriteCallBack(const boost::system::error_code& error
                            , size_t bytes_transferred
                            , std::shared_ptr<MsgNode> msg);
    void WriteToSocket(const std::string& buf);

private:
    std::queue<std::shared_ptr<MsgNode>> _send_queue;
    std::shared_ptr<asio::ip::tcp::socket> _socket;
    std::shared_ptr<MsgNode> _send_node;
    bool _send_pending;
};