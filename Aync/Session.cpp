#include "Session.h"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket)
    :_socket(socket), _send_pending(false), _recv_pending(0){
}

void Session::Connect(const asio::ip::tcp::endpoint& endpoint){
    _socket->connect(endpoint);
}

// 核心逻辑：处理部分发送 (Partial Write)
// TCP 是流式协议，async_write_some 可能只发送了缓冲区的一部分数据。
// 我们需要检查已发送字节数 (bytes_transferred) + 之前已发送的 (_cur_len) 是否等于总长度。
// 如果发送完成，shared_ptr<MsgNode> 引用计数减一，若为0则自动析构释放内存
void Session::WriteCallBackErr(const boost::system::error_code& ec, size_t bytes_transferred
    , std::shared_ptr<MsgNode> msg_node){
    
    if(ec.value() != 0){
        return;
    }
    if(bytes_transferred + msg_node->_cur_len < msg_node->_total_len){
        _send_node->_cur_len += bytes_transferred;
        
        this->_socket->async_write_some(asio::buffer(_send_node->_msg 
            + _send_node->_cur_len, _send_node->_total_len - _send_node->_cur_len),
            std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
    }
}

// 1. 创建 MsgNode，深拷贝数据。使用 shared_ptr 管理，防止回调前内存泄漏。
// 2. 发起异步写操作
// async_write_some 不能保证一次性发送所有数据，所以需要配合 WriteCallBackErr 处理部分发送。
// std::bind 将 _send_node 作为参数绑定到回调函数中，增加引用计数，确保在回调执行前 MsgNode 不会被销毁。
void Session::WriteToSocketErr(const std::string& buf){
    _send_node = make_shared<MsgNode>(buf.c_str(), buf.length());
    
    this->_socket->async_write_some(asio::buffer(_send_node->_msg, _send_node->_total_len),
        std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
}

void Session::WriteCallBack(const boost::system::error_code& ec, size_t bytes_transferred
    , std::shared_ptr<MsgNode> msg_node){

    if(ec.value() != 0){
        return;
    }

    auto &send_data = _send_queue.front();
    send_data->_cur_len += bytes_transferred;
    if(send_data->_cur_len < send_data->_total_len){
        this->_socket->async_write_some(asio::buffer(send_data->_msg + send_data->_cur_len,
            send_data->_total_len - send_data->_cur_len),
            std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
        return;
    }
    
    _send_queue.pop();

    if(_send_queue.empty()){
        _send_pending = false;
    }

    if (!_send_queue.empty()) {
        auto& send_data = _send_queue.front();
        this->_socket->async_write_some(asio::buffer(send_data->_msg + _send_node->_cur_len, 
            send_data->_total_len - send_data->_cur_len),
            std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
    }
    
}
void Session::WriteToSocket(const std::string& buf){
    _send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
    if(_send_pending){
        return;
    }

    this->_socket->async_write_some(asio::buffer(buf), 
        std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));

    _send_pending = true;
}

void Session::WriteAllToSocket(const std::string& buf){
    _send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
    if(_send_pending){
        return;
    }

    this->_socket->async_send(asio::buffer(buf), 
        std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _send_pending = true;
}

void Session::WriteAllCallBack(const boost::system::error_code& error, size_t bytes_transferred
    , std::shared_ptr<MsgNode> msg){
    if(error.value() != 0){
        return;
    }

    _send_queue.pop();
    if(_send_queue.empty()){
        _send_pending = false;
        return;
    }

    if(!_send_queue.empty()){
    auto& send_data = _send_queue.front();
    this->_socket->async_send(asio::buffer(send_data->_msg + send_data->_cur_len, 
        send_data->_total_len - send_data->_cur_len),
        std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void Session::ReadFromSocket(){
    if(_recv_pending){
        return;
    }

    _recv_node = std::make_shared<MsgNode>(RECVSIZE);
    this->_socket->async_read_some(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
        std::bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::ReadCallBack(const boost::system::error_code& error, size_t bytes_transferred){
    _recv_node->_cur_len += bytes_transferred;
    if(_recv_node->_cur_len < _recv_node->_total_len){
        _socket->async_read_some(asio::buffer(_recv_node->_msg + _recv_node->_cur_len, 
            _recv_node->_total_len - _recv_node->_cur_len),
            std::bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));

        return;
    }

    _recv_pending = 0;
    _recv_node = nullptr;
}