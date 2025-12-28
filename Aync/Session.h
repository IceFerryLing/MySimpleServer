#pragma once
#include <memory>
#include "boost/asio.hpp"
using namespace boost;
using namespace std;


class Session{
public:
    Session(std::shared_ptr<asio::ip::tcp::socket> socket);
    void Connect(const asio::ip::tcp::endpoint& endpoint);

private:
    std::shared_ptr<asio::ip::tcp::socket> socket;
};