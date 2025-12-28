// 引入必要的头文件
#include <boost/asio.hpp>   // Boost.Asio核心库，提供网络编程接口
#include <iostream>         // 标准输入输出流

// 新增：Windows特有的Winsock头文件
// 在Windows平台，使用Socket前必须初始化Winsock库
#include <winsock2.h>       // Windows Socket API 2.0
#include <cstring>          // C风格字符串操作函数

// 使用命名空间简化代码
using namespace boost::asio::ip;  // 使用boost::asio::ip命名空间下的TCP、IP相关类
using namespace std;              // 使用标准库命名空间

// 定义缓冲区最大长度
const int MAX_LENGTH = 1024;      // 限制单次发送/接收数据的最大长度

/**
 * @brief 主函数 - TCP客户端实现
 * 
 * 功能：连接到指定服务器，发送用户输入的消息，接收服务器回复
 * 特点：同步阻塞I/O，Windows平台需要Winsock初始化
 * 
 * @return 程序退出码（0=成功，-1=失败）
 */
int main(){
    try{
        // ==================== Windows特有的初始化 ====================
        // 注意：在Windows平台使用Socket前必须初始化Winsock库
        WSADATA wsaData;                     // 存储Winsock库信息
        
        // 初始化Winsock 2.2版本
        // WSAStartup函数加载Winsock DLL并初始化网络子系统
        // MAKEWORD(2,2)指定使用Winsock 2.2版本
        // wsaData用于接收库的详细信息
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        // =============================================================

        // **********************学习重点：IO上下文（☆☆☆☆☆）**********************
        // 作用：所有异步I/O操作的核心，管理所有I/O事件和回调
        // 类比：交通调度中心，协调所有车辆的运行
        boost::asio::io_context ioc;

        // 创建服务器端点：IP地址 + 端口号
        // make_address：将字符串IP转换为地址对象
        // 127.0.0.1：环回地址，指向本机，用于本地测试
        // 10086：服务器监听的端口号
        tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);

        // 创建TCP套接字对象
        // socket：网络通信的端点，类似电话听筒
        // 参数：ioc - 套接字关联的IO上下文
        tcp::socket socket(ioc);
        
        // **********************学习重点：连接服务器（☆☆☆☆☆）**********************
        // 方法：使用带错误码的connect，避免异常
        // 作用：发起TCP三次握手，建立到服务器的连接
        boost::system::error_code ec;  // 错误码容器
        
        // 连接服务器（阻塞操作，直到连接成功或失败）
        socket.connect(remote_ep, ec);
        
        // 检查连接是否成功
        if(ec){
            // 连接失败的原因可能包括：
            // 1. 服务器未运行
            // 2. 防火墙阻挡
            // 3. 网络不通
            // 4. 端口错误
            cout << "connect error,code is:" << ec.value() 
                << ", message: " << ec.message() << endl;
            
            // 清理Winsock资源（必须与WSAStartup配对调用）
            WSACleanup();
            return -1;  // 返回错误码
        }

        // 连接成功提示
        std::cout << "connected to server successfully." << std::endl;

        // ==================== 准备发送数据 ====================
        char request[MAX_LENGTH];       // 发送缓冲区
        
        // 从控制台读取用户输入
        // cin.getline：读取一行输入，包含空格
        // MAX_LENGTH：限制输入长度，防止缓冲区溢出
        std::cin.getline(request, MAX_LENGTH);
        
        // 获取实际输入字符串的长度（不包括结尾的空字符）
        size_t request_length = strlen(request);
        
        // **********************学习重点：数据发送（☆☆☆☆☆）**********************
        // 使用asio::write自由函数发送数据
        // 特点：
        // 1. 阻塞操作：直到所有数据发送完成
        // 2. 自动处理部分写：内部会循环发送直到完成
        // 3. 保证数据完整性：要么全部发送，要么失败
        boost::asio::write(socket, boost::asio::buffer(request, request_length));
        
        // 显示发送的消息
        std::cout << "sent message to server: ";
        std::cout.write(request, request_length);  // 原始输出，包含特殊字符
        std::cout << "\n";

        // ==================== 接收服务器回复 ====================
        char reply[MAX_LENGTH];  // 接收缓冲区
        
        // **********************学习重点：数据接收（☆☆☆☆☆）**********************
        // 使用asio::read自由函数接收数据
        // 特点：
        // 1. 阻塞操作：直到读取指定数量的字节
        // 2. 自动处理部分读：内部循环直到读取指定长度
        // 3. 参数：从socket读取数据到reply缓冲区，读取request_length个字节
        size_t reply_length = boost::asio::read(
            socket, 
            boost::asio::buffer(reply, request_length)  // 读取与发送数据相同长度的回复
        );
        
        // 显示服务器回复
        // string(reply, reply_length)：将字符数组转换为字符串
        cout << "server reply: " << string(reply, reply_length) << endl;

        // ==================== 清理资源 ====================
        // 注意：程序正常退出前必须清理Winsock
        WSACleanup();
        
    }catch(std::exception& e){
        // 异常处理：捕获标准异常和Boost.Asio异常
        // 可能发生的异常：
        // 1. IP地址格式错误
        // 2. 内存分配失败
        // 3. 其他运行时错误
        std::cerr << "error: " << e.what() << std::endl;
        
        // 异常时也需要清理Winsock
        WSACleanup();
    }
    
    return 0;  // 程序正常退出
}