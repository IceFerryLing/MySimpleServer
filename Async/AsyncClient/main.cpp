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

        // 启动IO线程 (负责接收和异步发送)
        thread t([&ioc]() {
            ioc.run();
        });

        // 启动发送线程 (模拟高频发送)
        // 依照模版：每2ms发送一次 "hello world!"
        thread send_thread([&client]() {
            while (true) {
                this_thread::sleep_for(std::chrono::milliseconds(2));
                client.Send("hello world!");
            }
        });

        // 主线程等待
        t.join();
        send_thread.join();
        
        WSACleanup();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}
