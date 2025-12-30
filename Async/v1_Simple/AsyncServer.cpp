#define _POSIX_SEM_VALUE_MAX 32767 // Fix for MinGW GCC 15 bug
#include <iostream>
#include <boost/asio.hpp>
#include "Session_demo.h"
#include "Server_demo.h"

int main(){
    try{
        boost::asio::io_context io_context;
        Server server(io_context, 12345);
        io_context.run();
    }catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
