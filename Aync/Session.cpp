#include "Session.h"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket)
    :socket(socket){
}

void Session::Connect(const asio::ip::tcp::endpoint& endpoint){
    socket->connect(endpoint);
}