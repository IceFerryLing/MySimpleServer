// 引入必要的头文件
#include <iostream>           // 标准输入输出流
#include <boost/asio.hpp>     // Boost.Asio网络编程库
#include <set>               // 集合容器，用于存储线程指针
#include <memory>            // 智能指针相关
#include <thread>            // C++11线程支持库

// 使用命名空间简化代码
using boost::asio::ip::tcp;  // 简化TCP相关类的使用

// 定义常量
const int MAX_LENGTH = 1024;  // 消息最大长度（防止缓冲区溢出）

// 类型别名定义
typedef std::shared_ptr<tcp::socket> socket_ptr;  // 套接字智能指针
std::set<std::shared_ptr<std::thread>> thread_set;  // 线程集合（用于跟踪所有线程）
using namespace std;  // 使用标准库命名空间

/**
 * @brief 客户端会话处理函数
 * 
 * 功能：处理单个客户端的完整通信会话
 * 特点：每个客户端连接在一个独立线程中运行
 * 
 * @param sock 客户端套接字的智能指针
 */
void session(socket_ptr sock){
    try{
        // 循环处理客户端请求（直到连接关闭）
        for(;;){
            // **********************学习重点：数据接收准备（☆☆☆☆）**********************
            // 创建接收缓冲区并初始化为0
            // 这样做可以避免脏数据问题和缓冲区溢出风险
            char data[MAX_LENGTH];
            memset(data, '\0', MAX_LENGTH);  // 清零缓冲区
            
            // 错误码容器（避免使用异常处理常规错误）
            boost::system::error_code error;
            
            // **********************学习重点：read_some函数（☆☆☆☆☆）**********************
            // read_some特点：
            // 1. 非阻塞或部分读：只读取当前可用的数据
            // 2. 可能返回少于请求字节数的数据
            // 3. 配合循环使用可以处理完整消息
            size_t length = sock->read_some(boost::asio::buffer(data, MAX_LENGTH), error);
            
            // 检查读取结果
            if(error == boost::asio::error::eof){
                // 客户端正常关闭连接（发送了FIN包）
                std::cout << "client disconnected." << std::endl;
                break;  // 跳出循环，结束会话
            }else if(error){
                // 其他读取错误（网络错误、连接重置等）
                throw boost::system::system_error(error);  // 抛出异常
            }

            // 显示客户端信息和接收的数据
            // remote_endpoint()：获取客户端地址信息
            // address().to_string()：将IP地址转换为字符串
            cout << "Listening to the message,IP is: " 
                << sock->remote_endpoint().address().to_string() << std::endl;
            cout << "data received: " << data << std::endl;

            // **********************学习重点：数据回显（☆☆☆☆☆）**********************
            // 将接收到的数据原样发送回客户端（echo服务器）
            // asio::write特点：阻塞操作，确保所有数据发送完成
            boost::asio::write(*sock, boost::asio::buffer(data, length));
            
            // 注意：这里没有处理粘包/拆包问题
            // 实际应用中需要定义消息边界（如长度前缀、分隔符等）
        }
    }catch(std::exception& e){
        // 会话异常处理（客户端异常断开、网络错误等）
        std::cerr << "error: " << e.what() << std::endl;
    }
    
    // 注意：当函数返回时，sock智能指针会析构，自动关闭套接字
}

/**
 * @brief 服务器主函数
 * 
 * 功能：监听指定端口，接受客户端连接
 * 特点：为每个客户端创建独立线程处理
 * 
 * @param io_context IO上下文（事件调度器）
 * @param port 监听端口号
 */
void server(boost::asio::io_context& io_context, unsigned short port){
    // **********************学习重点：创建接收器（☆☆☆☆☆）**********************
    // tcp::acceptor：专门用于接受连接的套接字
    // 参数：
    // 1. io_context：关联的IO上下文
    // 2. tcp::endpoint(tcp::v4(), port)：监听端点（所有IPv4地址 + 指定端口）
    tcp::acceptor acceptor (io_context, tcp::endpoint(tcp::v4(), port));
    
    // 无限循环，持续接受客户端连接
    for(;;){
        // 创建新的套接字对象（智能指针管理）
        // 每个客户端连接使用独立的套接字
        socket_ptr socket(new tcp::socket(io_context));
        
        // **********************学习重点：接受连接（☆☆☆☆☆）**********************
        // accept函数特点：
        // 1. 阻塞操作：等待客户端连接
        // 2. 接受连接后创建新的通信套接字
        // 3. 后续通信使用新套接字，acceptor继续监听
        acceptor.accept(*socket);  // 阻塞直到有客户端连接
        
        // 显示新客户端连接信息
        std::cout << "connected: " << socket->remote_endpoint().address().to_string() << std::endl;
        
        // **********************学习重点：创建线程处理客户端（☆☆☆☆☆）**********************
        // 为每个客户端创建独立线程
        // make_shared：创建线程的智能指针
        // session：线程入口函数
        // socket：传递给session的参数
        auto t = std::make_shared<std::thread>(session, socket);
        
        // 将线程指针存入集合（便于管理）
        thread_set.insert(t);
        
        // **********************学习重点：线程分离（☆☆☆☆）**********************
        // detach()特点：
        // 1. 分离线程：主线程不等待子线程结束
        // 2. 线程在后台独立运行
        // 3. 线程结束时自动清理资源
        // 4. 缺点：主线程无法控制子线程的执行
        t->detach();  // 让线程在后台运行
        
        // 注意：这里使用detach而不是join，因为服务器需要持续接受新连接
        // 如果使用join，服务器会在处理完一个客户端后才能接受下一个
    }
    // 注意：这个无限循环没有退出条件，服务器会一直运行
}

/**
 * @brief 主函数
 * 
 * 功能：启动服务器，管理程序生命周期
 * 
 * @return 程序退出码（0=成功）
 */
int main(){
    try{
        // **********************学习重点：IO上下文创建（☆☆☆☆☆）**********************
        // io_context是Asio的核心，管理所有I/O操作
        // 一个服务器通常只需要一个io_context
        boost::asio::io_context io_context;
        
        // **********************学习重点：服务器线程分离（☆☆☆☆）**********************
        // 将服务器运行在独立线程中，避免阻塞主线程
        // std::thread：创建新线程
        // std::ref(io_context)：传递io_context的引用
        std::thread server_thread(server, std::ref(io_context), 10086);
        
        // 分离服务器线程（让服务器在后台运行）
        server_thread.detach();
        
        // 显示服务器启动信息
        std::cout << "Listening to 10086" << endl;
        std::cout << "enter to stop" << endl;
        
        // **********************学习重点：主线程阻塞（☆☆☆）**********************
        // getchar()：等待用户输入（按回车键）
        // 这使得主线程不会立即退出，让服务器继续运行
        // 实际应用中可能使用信号处理或条件变量来控制服务器停止
        getchar();  // 阻塞主线程，直到用户按回车
        
        // 注意：由于服务器线程已经detach，这里的join无法执行
        // 实际上这段代码永远不会执行到下面的for循环
        for(auto& t : thread_set){
            if(t->joinable()){
                t->join();  // 等待所有线程结束（这里不会执行到）
            }
        }
        
    }catch(std::exception& e){
        // 捕获并显示异常
        std::cerr << "error: " << e.what() << "\n";
    }
    
    return 0;  // 程序正常退出
}