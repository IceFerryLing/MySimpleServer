#pragma once
#ifndef _POSIX_SEM_VALUE_MAX
#define _POSIX_SEM_VALUE_MAX 32767 // Fix for MinGW GCC 15 bug
#endif
#include <climits>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using boost::asio::ip::tcp;

class Server;

class Session:public enable_shared_from_this<Session>{
public:
    Session(boost::asio::io_context& ioc, Server* server):_socket(ioc), _server(server){
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }

    tcp::socket& Socket(){
        return _socket;
    }

    void Start();

    std::string& GetUuid();

private:
    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred, shared_ptr<Session> _self_shared);
    void HandleWrite(const boost::system::error_code& error, shared_ptr<Session> _self_shared);

    tcp::socket _socket;
    enum{max_length = 1024};
    char _data[max_length]; // v1 使用简单的 _data 缓冲区
    Server* _server;
    std::string _uuid;
};
