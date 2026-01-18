#include "MsgNode.h"
#include "const.h"
#include <cstring>
#include <iostream>
#include "LogicSystem.h"
using namespace std;

#define MAX_LENGTH 1024*2
#define HEAD_LENGTH 2
#define HEAD_TOTAL_LEN (HEAD_ID_LEN + HEAD_DATA_LEN)
// 构造函数：深拷贝数据到内部缓冲区 _msg
// msg: 待发送的数据
// total_len: 数据长度
MsgNode::MsgNode(const char* msg, int total_len):_total_len(total_len + HEAD_LENGTH), _cur_len(0){
        _msg = new char[_total_len + 1];             // 多分配1字节存放'\0'
        //转换为网络字节序
        int max_len_host = boost::asio::detail::socket_ops::host_to_network_short(total_len);
        memcpy(_msg, &max_len_host, HEAD_LENGTH);      // 复制消息头   
        memcpy(_msg + HEAD_LENGTH, msg, total_len);    // 复制消息体
        _msg[_total_len] = '\0';                        // 添加字符串结束符
    }

// 构造函数：分配指定长度的缓冲区
// total_len: 缓冲区大小
MsgNode::MsgNode(int total_len):_total_len(total_len + HEAD_LENGTH), _cur_len(0){
        _msg = new char[_total_len + 1];                 // 多分配1字节存放'\0'
    }


// 析构函数：释放内部缓冲区
MsgNode::~MsgNode(){
        delete[] _msg;
    }

void MsgNode::Clear(){
        ::memset(_msg, 0, _total_len);
        _cur_len = 0;
    }

RecvNode::RecvNode(short max_len, short msg_id)
    :MsgNode(max_len), _msg_id(msg_id){
}

SendNode::SendNode(const char* msg, short max_len, short msg_id)
    :MsgNode(max_len + HEAD_LENGTH), _msg_id(msg_id){
    //先发送id,转账为网络字节序
    //host_to_network_short是Boost.Asio库中的一个函数，用于将主机字节序转换为网络字节序（大端序）。
    short msg_id_net = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_msg, &msg_id_net, HEAD_ID_LEN);               // 复制消息ID
    //再发送数据长度
    short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_msg + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN); // 复制数据长度
    //最后发送数据体
    memcpy(_msg + HEAD_TOTAL_LEN, msg, max_len); // 复制消息体
}