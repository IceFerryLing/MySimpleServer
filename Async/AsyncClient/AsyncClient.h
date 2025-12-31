#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <queue>
#include <mutex>
#include <vector>

using namespace boost::asio::ip;
using namespace std;

class AsyncClient {
public:
    AsyncClient(boost::asio::io_context& ioc, const string& ip, int port);
    void Close();
    void Send(const string& msg);

private:
    void do_connect();
    void do_read_header();
    void do_read_body(short msglen);
    void do_write();

private:
    tcp::socket _socket;
    tcp::endpoint _endpoint;
    
    queue<vector<char>> _send_queue;
    
    enum { HEAD_LENGTH = 2, MAX_LENGTH = 1024 * 2 };
    char _recv_head[HEAD_LENGTH];
    vector<char> _recv_msg;
};
