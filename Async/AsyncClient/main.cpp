#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <winsock2.h> 
#include "AsyncClient.h"

using namespace std;

int main() {
    try {
        // Windows下必须初始化WSA
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        boost::asio::io_context ioc;
        
        // 创建客户端
        AsyncClient client(ioc, "127.0.0.1", 10086);

        // 启动IO线程
        thread t([&ioc]() {
            ioc.run();
        });

        // 主线程处理输入
        std::cout << "Enter message: ";
        while (true) {
            char request[1024];
            std::cin.getline(request, 1024);
            if (strcmp(request, "quit") == 0) break;
            
            client.Send(request);
        }

        client.Close();
        t.join();
        WSACleanup();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}
