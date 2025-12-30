#include "Session_demo.h"
#include "Server_demo.h"
#include <iostream>
#include <cstring> // For memset
using namespace std;

void Session::Start(){
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
        std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, shared_from_this()));
}

std::string& Session::GetUuid(){
    return _uuid;
}

// v1 版本：半双工模式 (Half-Duplex)
// 读 -> 写 -> 读 -> 写
// 缺点：
// 1. 必须等写完才能读下一条，吞吐量低。
// 2. 如果写操作耗时，会阻塞接收。
// 3. 直接使用 _data 缓冲区发送，如果逻辑不严谨（如并发读写），数据会损坏。
void Session::HandleRead(const boost::system::error_code& error, 
    size_t bytes_transferred, shared_ptr<Session> _self_shared){
    if(!error){
        cout << "Received from client: " << _socket.remote_endpoint().address().to_string() << endl;
        cout << "Received data: ";
        cout.write(_data, bytes_transferred);
        cout << endl;

        // 收到数据后，立即回显 (Echo)
        // 注意：这里直接使用了 _data 缓冲区，因为在 HandleWrite 之前我们不会发起新的 Read，
        // 所以 _data 不会被覆盖。这是安全的，但是是半双工的。
        boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
            std::bind(&Session::HandleWrite, this, placeholders::_1, _self_shared));
    }else{
        cerr << "Read error: " << error.message() << endl;
        _server->ClearSession(_uuid);
    }
}

void Session::HandleWrite(const boost::system::error_code& error, 
    shared_ptr<Session> _self_shared){
    if(!error){
        memset(_data, 0, max_length);
        
        // 发送完成后，才发起下一次读取
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
            std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));

    }else{
        cerr << "Write error: " << error.message() << endl;
        _server->ClearSession(_uuid);
    }
}
