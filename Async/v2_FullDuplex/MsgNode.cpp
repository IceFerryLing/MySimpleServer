#include "MsgNode.h"
#include <cstring>
#include <iostream>
using namespace std;

// 构造函数：深拷贝数据到内部缓冲区 _msg
// msg: 待发送的数据
// total_len: 数据长度
MsgNode::MsgNode(const char* msg, int total_len):_total_len(total_len), _cur_len(0){
        _msg = new char[total_len];
        memcpy(_msg, msg, total_len);
    }

// 构造函数：分配指定长度的缓冲区
// total_len: 缓冲区大小
MsgNode::MsgNode(int total_len):_total_len(total_len), _cur_len(0){
        _msg = new char[total_len];
    }


// 析构函数：释放内部缓冲区
MsgNode::~MsgNode(){
        delete[] _msg;
    }