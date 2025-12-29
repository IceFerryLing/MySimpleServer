#include "Session_demo.h"
#include <iostream>
using namespace std;

void Session::Start(){
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
        std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, shared_from_this()));
}

std::string& Session::GetUuid(){
    return _uuid;
}

void Session::handle_read(const boost::system::error_code& error, 
    size_t bytes_transferred, shared_ptr<Session> _self_shared){
    if(!error){
        cout << "Received from client: " << _socket.remote_endpoint().address().to_string() << endl;
        cout << "Received data: " << _data << endl;

        boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
            std::bind(&Session::handle_write, this, placeholders::_1, _self_shared));
    }else{
        cerr << "Read error: " << error.message() << endl;
        _server->ClearSession(_uuid);
        // delete this; // 不需要手动 delete，shared_ptr 会自动管理
    }
}

void Session::handle_write(const boost::system::error_code& error, 
    shared_ptr<Session> _self_shared){
    if(!error){
        memset(_data, 0, max_length);

        _socket.async_read_some(boost::asio::buffer(_data, max_length),
            std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));

    }else{
        cerr << "Write error: " << error.message() << endl;
        _server->ClearSession(_uuid);
        // delete this; // 不需要手动 delete，shared_ptr 会自动管理
    }
}


