#include <boost/asio.hpp>
#include <iostream>
// 新增：Windows必须的头文件
#include <winsock2.h>
#include <cstring>

using namespace boost::asio::ip;
using namespace std;
const int MAX_LENGTH = 1024;

int main(){
    try{
        // 新增：初始化Windows Socket（必须）
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        boost::asio::io_context ioc;
        tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);
        tcp::socket socket(ioc);
        
        // 改动1：删掉重复的connect，只保留1次带错误码的connect
        boost::system::error_code ec;
        socket.connect(remote_ep, ec);
        if(ec){
            cout << "connect error,code is:" << ec.value() << endl;
            WSACleanup(); // 新增：失败时清理WSA
            return -1;
        }

        std::cout << "Connected to server successfully." << std::endl;
        char request[MAX_LENGTH];
        std::cin.getline(request, MAX_LENGTH);
        size_t request_length = strlen(request);
        
        boost::asio::write(socket, boost::asio::buffer(request, request_length));
        std::cout << "Sent message to server: " ;
        std::cout.write(request, request_length);
        std::cout << "\n";

        char reply[MAX_LENGTH];
        // 可选：取消注释，接收服务端回传（不影响连接，只是能看到交互）
        size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, request_length));
        cout << "The reply:" << string(reply, reply_length) << endl;

        // 新增：清理WSA
        WSACleanup();
    }catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
        WSACleanup(); // 新增：异常时清理
    }
    return 0;
}