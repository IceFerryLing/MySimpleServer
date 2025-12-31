#include "MsgNode.h"
#include <cstring>
#include <iostream>
using namespace std;

#define MAX_LENGTH 1024*2
#define HEADE_LENGTH 2

// 构造函数：深拷贝数据到内部缓冲区 _msg
// msg: 待发送的数据
// total_len: 数据长度
MsgNode::MsgNode(const char* msg, int total_len):_total_len(total_len + HEADE_LENGTH), _cur_len(0){
        _msg = new char[_total_len + 1];             // 多分配1字节存放'\0'
        memcpy(_msg, &total_len, HEADE_LENGTH);      // 复制消息头   
        memcpy(_msg + HEADE_LENGTH, msg, total_len); // 复制消息体
        _msg[_total_len] = '\0';                     // 添加字符串结束符
    }

// 构造函数：分配指定长度的缓冲区
// total_len: 缓冲区大小
MsgNode::MsgNode(int total_len):_total_len(total_len + HEADE_LENGTH), _cur_len(0){
        _msg = new char[_total_len + 1];             // 多分配1字节存放'\0'
    }


// 析构函数：释放内部缓冲区
MsgNode::~MsgNode(){
        delete[] _msg;
    }

void MsgNode::Clear(){
        ::memset(_msg, 0, _total_len);
        _cur_len = 0;
    }