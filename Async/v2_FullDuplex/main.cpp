//优雅退出程序

#include <iostream>
#include "Server_demo.h"
#include <csignal>
#include <thread>
#include <mutex>

using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

void signal_handler(int sig){
    if(sig == SIGINT || sig == SIGTERM){
        std::unique_lock<std::mutex> lock_quit(mutex_quit);
        bstop = true;               //设置退出标志
        cond_quit.notify_all();     //通知等待线程退出
    }
}


int main(){
    try{
        boost::asio::io_context io_context;
        //启动网络线程,运行io_context
        std::thread net_work_thread([&io_context]{
            Server s(io_context, 10086);
            io_context.run();
        });
        
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

        while(!bstop){
            std::unique_lock<std::mutex> lock_quit(mutex_quit);
            cond_quit.wait(lock_quit);
        }
        
        io_context.stop();      //停止io_context运行
        net_work_thread.join(); //等待网络线程退出
    }catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    boost::asio::io_context io_context;
}