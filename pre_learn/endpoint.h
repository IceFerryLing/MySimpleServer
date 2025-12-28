/**
 * @file endpoint.h
 * @brief Boost.Asio 网络编程基础函数声明
 * 
 * 这个头文件声明了网络编程的核心操作函数，涵盖客户端和服务器的基本流程
 */

#pragma once 

/**
 * @brief 客户端端点创建
 * 
 * 将IP字符串(如"127.0.0.1")转换为地址对象，并与端口组合成端点
 * @return 0=成功，非0=错误码
 */
extern int client_end_point();

/**
 * @brief 服务器端点创建
 * 
 * 创建服务器监听端点，监听所有IPv4地址(0.0.0.0)和指定端口
 * @return 0=成功
 */
extern int server_end_point();

/**
 * @brief 创建TCP套接字
 * 
 * 创建并打开一个TCP套接字（网络通信的基本单元）
 * @return 0=成功，非0=错误码
 */
extern int create_socket();

/**
 * @brief 创建TCP接收器
 * 
 * 创建服务器专用的接收器，用于接受客户端连接
 * @return 0=成功，非0=错误码
 */
extern int create_acceptor_socket();

/**
 * @brief 绑定接收器到端口
 * 
 * 将接收器绑定到特定IP和端口（服务器声明端口使用权）
 * @return 0=成功，非0=错误码
 */
extern int bind_acceptor_socket();

/**
 * @brief 连接到远端服务器
 * 
 * 客户端发起连接请求，执行TCP三次握手
 * @return 0=成功，非0=错误码
 */
extern int connect_to_end();

/**
 * @brief 通过域名连接服务器
 * 
 * 使用域名而非IP地址连接服务器，自动进行DNS解析
 * @return 0=成功，非0=错误码
 */
extern int dns_connect_to_end();

/**
 * @brief 接受客户端连接
 * 
 * 服务器等待并接受客户端连接（阻塞操作）
 * @return 0=成功，非0=错误码
 */
extern int accept_new_connection();

extern void use_const_buffer();

extern void use_buffer_str();

extern void use_buffer_array();