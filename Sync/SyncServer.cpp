#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
#include <thread> // 确保包含线程头文件

using boost::asio::ip::tcp;

const int MAX_LENGTH = 1024;

typedef std::shared_ptr<tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;
using namespace std;

void session(socket_ptr sock){
    try{
        for(;;){
            char data[MAX_LENGTH];
            memset(data, '\0', MAX_LENGTH);
            boost::system::error_code error;
            size_t length = sock->read_some(boost::asio::buffer(data, MAX_LENGTH), error);
            if(error == boost::asio::error::eof){
                std::cout << "Connection closed by peer." << std::endl;
                break; 
            }else if(error){
                throw boost::system::system_error(error);
            }

            cout << "Received from client: " << sock->remote_endpoint().address().to_string() << std::endl;
            cout << "Received data: " << data << std::endl;

            boost::asio::write(*sock, boost::asio::buffer(data, length));
        }
    }catch(std::exception& e){
        std::cerr << "Session error: " << e.what() << std::endl;
    }
}

void server(boost::asio::io_context& io_context, unsigned short port){
    tcp::acceptor acceptor (io_context, tcp::endpoint(tcp::v4(), port));
    for(;;){
        socket_ptr socket(new tcp::socket(io_context));
        acceptor.accept(*socket);

        auto t = std::make_shared<std::thread>(session, socket);
        thread_set.insert(t);
        t->detach(); // 改动1：新增detach，让线程后台运行，不阻塞
    }
}

int main(){
    try{
        boost::asio::io_context io_context;
        // 改动2：把server放到独立线程运行，避免主线程卡死
        std::thread server_thread(server, std::ref(io_context), 10086);
        server_thread.detach();

        std::cout << "Listening on port 10086" << endl; // 新增：提示监听成功
        getchar(); // 新增：阻塞主线程，让服务端一直运行

        for(auto& t : thread_set){
            if(t->joinable()){
                t->join();
            }
        }
    }catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}