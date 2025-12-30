#pragma once
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

using namespace std;

// 前向声明 Session 类，因为它是 friend
class Session;

class MsgNode{
public:
    // 构造函数：深拷贝数据到内部缓冲区
    // msg: 待发送的数据
    // total_len: 数据长度
    MsgNode(const char* msg, int total_len);

    // 构造函数：仅分配空间，用于接收数据
    // total_len: 缓冲区大小
    MsgNode(int total_len);

    ~MsgNode();
    
    // 允许 Session 类访问私有成员
    friend class Session;

private:
    int _total_len; // 消息总长度
    int _cur_len;   // 当前已发送长度
    char* _data;    // 消息数据缓冲区
};