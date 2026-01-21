#include <iostream>
#include <boost/asio.hpp>
#include "Session_demo.h"
#include "Server_demo.h"
#include "const.h"
#include "json/json.h"

#define HEAD_LENGTH 2
using namespace std;
using namespace boost::asio::ip;


int main(){
    try{
        //创建io_context对象，负责管理异步操作的事件循环
        boost::asio::io_context io_context;
        //定义服务器监听的端口和地址
        tcp::endpoint remote_ep(tcp::v4(), 10086);
        tcp::socket socket(io_context);

        boost::system::error_code ec = boost::asio::error::host_not_found;
        socket.connect(remote_ep, ec);

        if(ec){
            cerr << "Connect failed: " << ec.message() << endl;
            return 0;
        }

        Json::Value root;
        root["id"] = "1001";
        root["name"] = "example";
        root["data"] = "Hello, World!";
        std::string request = root.toStyledString();
        size_t request_length = request.length();
        char send_buffer[1024] = {0};
        int msgid = 1001;
        int msgid_host = boost::asio::detail::socket_ops::host_to_network_short(msgid);
        memcpy(send_buffer, &msgid_host, HEAD_LENGTH);

        //消息长度转为网络字节序
        int request_host_length = boost::asio::detail::socket_ops::host_to_network_short(request_length);
        memcpy(send_buffer, &request_host_length, HEAD_LENGTH);
        memcpy(send_buffer + HEAD_LENGTH, request.c_str(), request_length);

        boost::asio::write(socket, boost::asio::buffer(send_buffer, request_length + HEAD_LENGTH));
        cout << "Sent request: " << request << endl;
        
        char reply_head[HEAD_LENGTH] = {0};
        size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply_head, HEAD_LENGTH));
        msgid = 0;
        memcpy(&msgid, reply_head, HEAD_LENGTH);
        short msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        //转为本地字节序
        msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
        msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);

        char msg[MAX_LENGTH] = {0};
        size_t len = boost::asio::read(socket, boost::asio::buffer(msg, msglen));
        Json::Reader reader;
        reader.parse(std::string(msg, len), root);
        cout << "Received reply: " << root.toStyledString() << endl;
        getchar();
        
    }catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}