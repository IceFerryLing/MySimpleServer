/**
 * @file endpoint.cpp
 * @brief 客户端和服务端网络编程基础示例
 */
#include "endpoint.h"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;
namespace asio = boost::asio;

/**
 * @brief 使用const_buffer创建缓冲区
 * 
 * const_buffer表示只读缓冲区，用于发送数据
 * 特点：数据不会被修改，适合发送固定数据
 * 原理：asio::buffer()函数将数据包装为Asio缓冲区对象，不复制数据
 */
void use_const_buffer(){
    std::string buf = "Hello world";
    // 将字符串包装为只读缓冲区
    asio::const_buffer asio_buf(buf.c_str(), buf.length());
    
    // 创建缓冲区序列，可以同时发送多个缓冲区
    std::vector<asio::const_buffer> buffers_sequeuence;
    buffers_sequeuence.push_back(asio_buf);
}

/**
 * @brief 使用mutable_buffer包装字符串
 * 
 * mutable_buffer表示可读写缓冲区，用于接收或修改数据
 * 特点：可以修改缓冲区内容
 * 警告：const_cast不安全，除非确定要修改只读数据
 */
void use_buffer_str(){
    std::string buf = "Hello world";
    // 将字符串转换为可读写缓冲区（使用const_cast移除const性）
    asio::mutable_buffer asio_buf(const_cast<char*>(buf.c_str()), buf.length());
    
    std::vector<asio::mutable_buffer> buffers_sequeuence;
    buffers_sequeuence.push_back(asio_buf);
}

/**
 * @brief 使用动态数组创建缓冲区
 * 
 * 使用unique_ptr管理动态分配的缓冲区内存
 * 特点：内存自动管理，适合不确定大小的缓冲区
 */
void use_buffer_array(){
    const int BUF_SIZES = 20;
    // 使用unique_ptr智能指针管理动态数组
    std::unique_ptr<char[]>buf(new char[BUF_SIZES]);

    // 将动态数组包装为缓冲区
    auto input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZES);
}

/**
 * @brief 使用write_some函数发送数据
 * 
 * write_some特点：
 * 1. 非阻塞或部分写：可能只发送部分数据
 * 2. 需要手动循环确保所有数据发送完成
 * 3. 返回实际发送的字节数
 * 
 * @param socket 已连接的套接字引用
 */
void write_to_socket(asio::ip::tcp::socket& socket){
    std::string buf = "hello world";
    std::size_t total_bytes_written = 0;

    // 循环发送直到所有数据发送完毕
    // write_some可能只发送部分数据（特别是非阻塞模式）
    while(total_bytes_written != buf.length()){
        total_bytes_written += socket.write_some(
            asio::buffer(buf.c_str() + total_bytes_written, 
                        buf.length() - total_bytes_written));
    }
}

/**
 * @brief 使用write_some发送数据到服务器
 * 
 * 完整流程：创建端点→连接→发送数据
 * 调用函数：write_to_socket内部使用write_some
 * @return 0=成功，非0=错误码
 */
int send_data_by_write_some(){
    std::string raw_ip_address = "192.168.3.114";  // 服务器IP
    unsigned short port_num = 3333;
    try{
        // 1. 创建服务器端点
        asio::ip::tcp::endpoint endpoint(
            asio::ip::make_address(raw_ip_address), 
            port_num
        );
        
        // 2. 创建IO上下文和套接字
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, endpoint.protocol());
        
        // 3. 连接服务器
        socket.connect(endpoint);
        
        // 4. 使用write_some发送数据
        write_to_socket(socket);
        return 0;

    }catch(const system::system_error& e){
        std::cout << "连接异常: " << e.what() << std::endl;
        return e.code().value();
    }
}

/**
 * @brief 使用send函数发送数据
 * 
 * send函数特点：
 * 1. 阻塞操作：直到所有数据发送完成或发生错误
 * 2. 自动处理部分写：内部循环直到数据全部发送
 * 3. 返回实际发送的总字节数
 * 
 * @return 0=成功，非0=错误码
 */
int send_data_by_send(){
    std::string raw_ip_address = "192.168.3.114";
    unsigned short port_num = 3333;
    try{
        // 创建端点、套接字并连接
        asio::ip::tcp::endpoint endpoint(
            asio::ip::make_address(raw_ip_address), 
            port_num
        );
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, endpoint.protocol());
        socket.connect(endpoint);
        
        // 准备要发送的数据
        std::string buf = "hello world";
        
        // 使用send发送数据（阻塞，自动处理部分写）
        int send_length = socket.send(
            asio::buffer(buf.c_str(), buf.length())
        );
        
        if(send_length <= 0){
            std::cout << "数据发送失败" << std::endl;
            return -1;
        }

    }catch(const system::system_error& e){
        std::cout << "连接异常: " << e.what() << std::endl;
        return e.code().value();
    }
}

/**
 * @brief 使用asio::write自由函数发送数据
 * 
 * asio::write特点：
 * 1. 自由函数，不是成员函数
 * 2. 阻塞操作：确保所有数据都写入套接字
 * 3. 自动处理部分写
 * 4. 可以写入多个缓冲区
 * 
 * @return 0=成功，非0=错误码
 */
int send_data_by_write(){
    std::string raw_ip_address = "192.168.3.114";
    unsigned short port_num = 3333;
    try{
        // 创建端点、套接字并连接
        asio::ip::tcp::endpoint endpoint(
            asio::ip::make_address(raw_ip_address), 
            port_num
        );
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, endpoint.protocol());
        socket.connect(endpoint);
        
        // 准备数据
        std::string buf = "hello world";
        
        // 使用asio::write自由函数发送数据
        int send_length = asio::write(
            socket, 
            asio::buffer(buf.c_str(), buf.length())
        );
        
        if(send_length <= 0){
            std::cout << "数据发送失败" << std::endl;
            return -1;
        }

    }catch(const system::system_error& e){
        std::cout << "连接异常: " << e.what() << std::endl;
        return e.code().value();
    }
}

/**
 * @brief 使用read_some接收数据
 * 
 * read_some特点：
 * 1. 非阻塞或部分读：可能只读取部分可用数据
 * 2. 需要手动循环确保读取指定数量的数据
 * 3. 返回实际读取的字节数
 * 
 * @param socket 已连接的套接字引用
 * @return 读取的字符串
 */
std::string read_from_socket(asio::ip::tcp::socket& socket){
    const unsigned char MESSAGE_SIZE = 7;  // 期望接收的消息大小
    char buf[MESSAGE_SIZE];
    std::size_t total_bytes_read = 0;
    
    // 循环读取直到收到指定数量的数据
    while(total_bytes_read != MESSAGE_SIZE){
        total_bytes_read += socket.read_some(
            asio::buffer(
                buf + total_bytes_read, 
                MESSAGE_SIZE - total_bytes_read
            )
        );
    }

    return std::string(buf, MESSAGE_SIZE);
}

/**
 * @brief 使用read_some接收数据
 * 
 * 完整流程：连接服务器→读取数据
 * 调用函数：read_from_socket内部使用read_some
 * @return 0=成功，非0=错误码
 */
int read_data_by_read_some(){
    std::string raw_ip_address = "127.0.0.1";  // 本地服务器
    unsigned short port_num = 3333;
    try{
        // 创建端点、套接字并连接
        asio::ip::tcp::endpoint endpoint(
            asio::ip::make_address(raw_ip_address), 
            port_num
        );
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, endpoint.protocol());
        socket.connect(endpoint);
        
        // 使用read_some读取数据
        read_from_socket(socket);
        return 0;
        
    }catch(const system::system_error& e){
        std::cout << "连接异常: " << e.what() << std::endl;
        return e.code().value();
    }
}

/**
 * @brief 使用receive函数接收数据
 * 
 * receive函数特点：
 * 1. 阻塞操作：直到缓冲区满或发生错误
 * 2. 自动处理部分读：内部可能循环读取
 * 3. 返回实际读取的字节数
 * 
 * @return 0=成功，非0=错误码
 */
int read_data_by_recieve(){
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;
    try{
        // 创建端点、套接字并连接
        asio::ip::tcp::endpoint endpoint(
            asio::ip::make_address(raw_ip_address), 
            port_num
        );
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, endpoint.protocol());
        socket.connect(endpoint);
        
        // 准备接收缓冲区
        const unsigned char BUFFER_SIZE = 7;
        char buffer_receive[BUFFER_SIZE];
        
        // 使用receive接收数据（阻塞，直到缓冲区满）
        int receive_length = socket.receive(
            asio::buffer(buffer_receive, BUFFER_SIZE)
        );
        
        if(receive_length <= 0){
            std::cout << "数据接收失败" << std::endl;
            return -1;
        }
        
    }catch(const system::system_error& e){
        std::cout << "连接异常: " << e.what() << std::endl;
        return e.code().value();
    }
}

/**
 * @brief 使用asio::read自由函数接收数据
 * 
 * asio::read特点：
 * 1. 自由函数，不是成员函数
 * 2. 阻塞操作：直到读取指定数量的数据
 * 3. 自动处理部分读
 * 4. 可以读取到多个缓冲区
 * 
 * @return 0=成功，非0=错误码
 */
int read_data_by_read(){
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;
    try{
        // 创建端点、套接字并连接
        asio::ip::tcp::endpoint endpoint(
            asio::ip::make_address(raw_ip_address), 
            port_num
        );
        asio::io_context ioc;
        asio::ip::tcp::socket socket(ioc, endpoint.protocol());
        socket.connect(endpoint);
        
        // 准备接收缓冲区
        const unsigned char BUFFER_SIZE = 7;
        char buffer_receive[BUFFER_SIZE];
        
        // 使用asio::read自由函数接收数据
        int receive_length = asio::read(
            socket,
            asio::buffer(buffer_receive, BUFFER_SIZE)
        );
        
        if(receive_length <= 0){
            std::cout << "数据接收失败" << std::endl;
            return -1;
        }
        
    }catch(const system::system_error& e){
        std::cout << "连接异常: " << e.what() << std::endl;
        return e.code().value();
    }
}

/**
 * @brief 使用read_until读取数据直到分隔符
 * 
 * read_until特点：
 * 1. 读取直到遇到指定分隔符（如'\n'）
 * 2. 返回的数据可能包含分隔符
 * 3. 使用streambuf动态管理内存
 * 4. 适合文本协议（如HTTP、自定义文本协议）
 * 
 * @param sock 已连接的套接字引用
 * @return 读取的字符串（不含分隔符）
 */
std::string read_data_by_until(asio::ip::tcp::socket& sock) {
    // asio::streambuf自动管理内存，适合变长数据
    asio::streambuf buf;  
    
    // 读取直到遇到换行符
    asio::read_until(sock, buf, '\n');
    
    std::string message; 
    std::istream input_stream(&buf);
    
    // 使用getline提取一行（不包含换行符）
    std::getline(input_stream, message);
    
    return message;
}