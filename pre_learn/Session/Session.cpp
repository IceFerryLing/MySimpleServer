#include "Session.h"

/**
 * @brief Session 构造函数
 * @param socket 已经连接的 socket 指针
 * @details 初始化 Session 对象，保存 socket 指针，初始化发送和接收状态
 */
Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket)
    :_socket(socket), _send_pending(false), _recv_pending(0){
}

/**
 * @brief 连接到指定端点
 * @param endpoint 目标 IP 和端口
 * @details 同步连接到服务器，连接成功后 socket 可用于读写
 */
void Session::Connect(const asio::ip::tcp::endpoint& endpoint){
    _socket->connect(endpoint);
}

/**
 * @brief 错误处理版本的写回调函数 (演示用)
 * @param ec 错误码
 * @param bytes_transferred 本次发送的字节数
 * @param msg_node 发送的消息节点，通过 shared_ptr 保持生命周期
 * @details 
 * 核心逻辑：处理 TCP 部分发送 (Partial Write)
 * TCP 是流式协议，async_write_some 可能只发送了缓冲区的一部分数据。
 * 我们需要检查已发送字节数 (bytes_transferred) + 之前已发送的 (_cur_len) 是否等于总长度。
 * 如果未发送完，继续发起异步写操作发送剩余部分。
 * 如果发送完成，shared_ptr<MsgNode> 引用计数减一，若为0则自动析构释放内存。
 */
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

/**
 * @brief 错误处理版本的发送函数 (演示用)
 * @param buf 要发送的字符串数据
 * @details 
 * 1. 创建 MsgNode，深拷贝数据。使用 shared_ptr 管理，防止回调前内存泄漏。
 * 2. 发起异步写操作 async_write_some。
 * 注意：此函数未处理并发调用问题，如果连续调用多次，可能会导致 socket 并发写错误。
 * 正确做法应该使用发送队列 (参考 WriteToSocket)。
 */
void Session::WriteToSocketErr(const std::string& buf){
    _send_node = make_shared<MsgNode>(buf.c_str(), buf.length());
    
    this->_socket->async_write_some(asio::buffer(_send_node->_msg, _send_node->_total_len),
        std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
}

/**
 * @brief 发送队列的写回调函数
 * @param ec 错误码
 * @param bytes_transferred 本次发送的字节数
 * @param msg_node (未使用) 这里的参数签名可能与 bind 不匹配，实际逻辑使用队列头部元素
 * @details 
 * 1. 检查错误。
 * 2. 更新队列头部消息的已发送长度。
 * 3. 如果当前消息未发送完，继续发送剩余部分。
 * 4. 如果当前消息发送完，将其从队列移除。
 * 5. 如果队列不为空，继续发送下一条消息。
 * 6. 如果队列为空，重置发送状态 _send_pending 为 false。
 */
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
/**
 * @brief 发送数据 (使用发送队列)
 * @param buf 要发送的字符串
 * @details 
 * 1. 将数据封装成 MsgNode 并放入发送队列 _send_queue。
 * 2. 检查当前是否已有发送任务在进行 (_send_pending)。
 * 3. 如果没有，则发起第一次异步写操作，并设置 _send_pending 为 true。
 * 4. 如果已有任务在进行，则只入队，由回调函数负责后续发送。
 * 这样保证了 socket 在同一时间只有一个异步写操作，符合 asio 要求。
 */
void Session::WriteToSocket(const std::string& buf){
    _send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
    if(_send_pending){
        return;
    }

    this->_socket->async_write_some(asio::buffer(buf), 
        std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));

    _send_pending = true;
}

/**
 * @brief 发送所有数据 (使用 async_send)
 * @param buf 要发送的字符串
 * @details 
 * 与 WriteToSocket 类似，但使用 async_send 替代 async_write_some。
 * async_send (底层通常调用 async_write) 会尝试发送完缓冲区所有数据才调用回调，
 * 简化了部分发送的处理逻辑。
 */
void Session::WriteAllToSocket(const std::string& buf){
    _send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
    if(_send_pending){
        return;
    }

    this->_socket->async_send(asio::buffer(buf), 
        std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _send_pending = true;
}

/**
 * @brief async_send 的回调函数
 * @param error 错误码
 * @param bytes_transferred 已发送字节数
 * @param msg (未使用)
 * @details 
 * 由于使用了 async_send，通常假设数据已全部发送（除非出错）。
 * 1. 移除队列头部的已发送消息。
 * 2. 如果队列仍有消息，发起下一次 async_send。
 * 3. 如果队列为空，重置发送状态。
 */
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

    auto& send_data = _send_queue.front();
    this->_socket->async_send(asio::buffer(send_data->_msg + send_data->_cur_len, 
        send_data->_total_len - send_data->_cur_len),
        std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2, send_data));
}

/**
 * @brief 从 socket 读取数据 (async_read_some)
 * @details 
 * 1. 创建接收缓冲区 _recv_node。
 * 2. 发起 async_read_some 异步读操作。
 * 注意：async_read_some 读取到任意长度数据就会返回，不保证填满缓冲区。
 */
void Session::ReadFromSocket(){
    if(_recv_pending){
        return;
    }

    _recv_node = std::make_shared<MsgNode>(RECVSIZE);
    this->_socket->async_read_some(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
        std::bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief 读回调函数
 * @param error 错误码
 * @param bytes_transferred 本次读取字节数
 * @details 
 * 1. 更新已接收长度。
 * 2. 这里的逻辑似乎是想填满整个缓冲区 (RECVSIZE) 才算结束？
 *    如果未填满，继续读取剩余部分。
 *    这通常用于读取定长消息头。
 * 3. 读取完成后，重置接收状态。
 */
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

/**
 * @brief 读取数据 (async_receive)
 * @details 
 * 使用 async_receive 发起读取。
 * async_receive 与 async_read_some 类似，但在某些系统上可能支持额外的标志 (flags)。
 */
void Session::ReadAllFromSocket(){
    if(_recv_pending){
        return;
    }

    _recv_node = std::make_shared<MsgNode>(RECVSIZE);
    this->_socket->async_receive(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
        std::bind(&Session::ReadAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief async_receive 的回调函数
 * @param error 错误码
 * @param bytes_transferred 本次读取字节数
 * @details 
 * 逻辑与 ReadCallBack 相同：循环读取直到填满缓冲区。
 */
void Session::ReadAllCallBack(const boost::system::error_code& error, size_t bytes_transferred){
    _recv_node->_cur_len += bytes_transferred;
    if(_recv_node->_cur_len < _recv_node->_total_len){
        _socket->async_receive(asio::buffer(_recv_node->_msg + _recv_node->_cur_len, 
            _recv_node->_total_len - _recv_node->_cur_len),
            std::bind(&Session::ReadAllCallBack, this, std::placeholders::_1, std::placeholders::_2));

        return;
    }

    _recv_pending = 0;
    _recv_node = nullptr;
}