#include "Session_demo.h"
#include "Server_demo.h"
#include <iostream>
using namespace std;


void Session::Start(){
    memset(_recv_buffer, 0, MAX_LENGTH);
    _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH),
        std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, shared_from_this()));
}

std::string& Session::GetUuid(){
    return _uuid;
}

void Session::Send(char* msg, int length){
    bool pending = false;// 是否有未完成的发送操作
    std::lock_guard<std::mutex> lock(_send_lock);
    if(!_send_queue.empty()){
        pending = true;
    }

    _send_queue.push(std::make_shared<MsgNode>(msg, length));
    if(pending){
        return;
    }

    boost::asio::async_write(_socket, boost::asio::buffer(msg, length), 
        std::bind(&Session::HandleWrite, this, placeholders::_1, shared_from_this()));
}

void Session::HandleRead(const boost::system::error_code& error, 
    size_t bytes_transferred, shared_ptr<Session> _self_shared){
    if(!error){
        cout << "Received from client: " << _socket.remote_endpoint().address().to_string() << endl;
        cout << "Received data: " << _recv_buffer << endl;

        boost::asio::async_write(_socket, boost::asio::buffer(_recv_buffer, bytes_transferred),
            std::bind(&Session::HandleWrite, this, placeholders::_1, _self_shared));
    }else{
        cerr << "Read error: " << error.message() << endl;
        _server->ClearSession(_uuid);
        // delete this; // 不需要手动 delete，shared_ptr 会自动管理
    }
}

void Session::HandleWrite(const boost::system::error_code& error, 
    shared_ptr<Session> _self_shared){
    if(!error){
        memset(_recv_buffer, 0, MAX_LENGTH);

        _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH),
            std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));

    }else{
        cerr << "Write error: " << error.message() << endl;
        _server->ClearSession(_uuid);
        // delete this; // 不需要手动 delete，shared_ptr 会自动管理
    }
}


