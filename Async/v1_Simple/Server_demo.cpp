#include "Server_demo.h"
#include <iostream>
#include <boost/asio.hpp>
using namespace std;

Server::Server(boost::asio::io_context& ioc, short port):_ioc(ioc)
    ,_acceptor(ioc, tcp::endpoint(tcp::v4(), port)){
    cout << "Server started on port: " << port << endl;
    StartAccept();
}

void Server::StartAccept(){
    shared_ptr<Session> new_session = make_shared<Session>(_ioc, this);

    _acceptor.async_accept(new_session->Socket(), std::bind(&Server::HandleAccept, this, new_session, std::placeholders::_1));
}

void Server::HandleAccept(shared_ptr<Session> new_session, const boost::system::error_code& error){
    if(!error){
        new_session->Start();
        _sessions.insert(make_pair(new_session->GetUuid(), new_session));
    }else{
        //delete new_session;
    }
    StartAccept();
}   

void Server::ClearSession(std::string uuid){
    _sessions.erase(uuid);
}
