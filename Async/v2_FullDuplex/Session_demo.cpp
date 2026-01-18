#include "Session_demo.h"
#include "Server_demo.h"
#include <iostream>
#include <iomanip>
#include "json/json.h"
#include "const.h"

using namespace std;

void Session::Start(){
    _recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
    memset(_recv_buffer, 0, MAX_LENGTH);
    _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH),
        std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, shared_from_this()));
}

void Session::Start(){
    _recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
    boost::asio::async_read(_socket, boost::asio::buffer(_recv_head_node->_msg, HEAD_TOTAL_LEN),
        std::bind(&Session::HandleReadHead, this, placeholders::_1, placeholders::_2, shared_from_this()));
}

void Session::Close(){
    _socket.close();
    _b_closed = true;
}

std::string& Session::GetUuid(){
    return _uuid;
}

//send函数将消息加入发送队列，并启动异步写操作
//参数是消息内容和长度
void Session::Send(std::string msg, short msg_id){
    int send_que_size = _send_queue.size();
    std::lock_guard<std::mutex> lock(_send_lock);
    if (send_que_size >= MAX_SENDQUE){
        std::cout << "Send queue full, dropping message" << std::endl;
        return;
    }
    _send_queue.push(std::make_shared<SendNode>(msg, msg.length(), msg_id));
    if(send_que_size > 0){
        return;
    }

    auto& msgnode = _send_queue.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_msg, msgnode->_total_len), 
        std::bind(&Session::HandleWrite, this, placeholders::_1, shared_from_this()));
}

void Session::Send(char* msg, int length){
    int send_que_size = _send_queue.size();
    std::lock_guard<std::mutex> lock(_send_lock);
    if (send_que_size >= MAX_SENDQUE){
        std::cout << "Send queue full, dropping message" << std::endl;
        return;
    }
    _send_queue.push(std::make_shared<SendNode>(msg, length));
    if(send_que_size > 0){
        return;
    }

    auto& msgnode = _send_queue.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_msg, msgnode->_total_len), 
        std::bind(&Session::HandleWrite, this, placeholders::_1, shared_from_this()));
}

void Session::HandleRead(const boost::system::error_code& error, 
    size_t bytes_transferred, shared_ptr<Session> _self_shared){
    
    if(!error){
        PrintRecvData(_recv_buffer, bytes_transferred);
        std::chrono::milliseconds dura(2000);
        std::this_thread::sleep_for(dura);
    }

    if(!error){
        //已经成功读取数据长度
        int copy_len = 0;
        while(bytes_transferred > 0){
            if (!_b_head_parsed){
                //收到的消息不足一个头的长度，继续读取
                if(bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN){
                    memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _recv_buffer + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    memset(_recv_buffer, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH),
                        std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
                    return;
                }

                //收到的数据足够一个头的长度
                //头部剩余为复制的data，和未处理的数据
                int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_msg + _recv_head_node->_cur_len, _recv_buffer + copy_len, head_remain);
                //更新复制位置和剩余未处理数据长度
                copy_len += head_remain;
                bytes_transferred -= head_remain;
                //解析头部获取消息ID和长度
                short msg_id = 0;
                memcpy(&msg_id, _recv_head_node->_msg, HEAD_ID_LEN);
                std::cout << " Parsed message ID: " << msg_id << endl;
                //网络字节序转换为主机字节序
                msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                cout << "Converted message ID: " << msg_id << endl;
                //id 
                if (msg_id > MAX_LENGTH){
                    // 消息ID超过最大限制，关闭会话
                    cerr << "Message ID exceeds maximum limit: " << msg_id << endl;
                    _server->ClearSession(_uuid);
                    return;
                }
                //获取头部解析消息长度
                short data_len = 0;
                memcpy(&data_len, _recv_head_node->_msg + HEAD_ID_LEN, HEAD_DATA_LEN);
                std::cout << "Parsed message length: " << data_len << endl;
                
                if (data_len > MAX_LENGTH){
                    // 消息长度超过最大限制，关闭会话
                    cerr << "Message length exceeds maximum limit: " << data_len << endl;
                    _server->ClearSession(_uuid);
                    return;
                }

                _recv_msg_node = make_shared<RecvNode>(data_len, msg_id);
                //剩余数据不足一个消息体，继续读取,将部分先放到消息体中
                if (bytes_transferred < data_len){
                    memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _recv_buffer + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;

                    memset(_recv_buffer, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH),
                        std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
                    
                    //头部处理完成
                    _b_head_parsed = true;
                    return;
                }

                memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _recv_buffer + copy_len, data_len);
                _recv_msg_node->_cur_len += data_len;
                copy_len += data_len;
                bytes_transferred -= data_len;
                _recv_msg_node->_msg[_recv_msg_node->_cur_len] = '\0';
                //头部处理完成
                std::cout << "Received from client: " << _socket.remote_endpoint().address().to_string() << std::endl;
                std::cout << "Received data: " << _recv_msg_node->_msg << std::endl;
                Json::Reader reader;
                Json::Value root;
                reader.parse(std::string(_recv_msg_node->_msg), root);
                std::cout << "received json data: " << root["id"].asInt()<< ", " 
                        << root["name"].asString() << ", "
                        << root["value"].asDouble() << std::endl;
                std::string return_str = root.toStyledString();
                Send(return_str.c_str(), return_str.size());
                //继续轮询剩余数据
                _b_head_parsed = false;
                _recv_head_node->Clear();

                if(bytes_transferred <= 0){
                    ::memset(_recv_buffer, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH),
                        std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    return;
                }
                continue;
            }
            
            //已经处理完头部，处理上次未接受完的消息数据
            //接收的数据仍不足剩余未处理的
            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            if (bytes_transferred < remain_msg) {
                memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _recv_buffer + copy_len, bytes_transferred);
                _recv_msg_node->_cur_len += bytes_transferred;
                ::memset(_recv_buffer, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH), 
                    std::bind(&Session::HandleRead, this, placeholders::_1, placeholders::_2, _self_shared));
                return;
            }
            //接收的数据足够剩余未处理的
            memcpy(_recv_msg_node->_msg + _recv_msg_node->_cur_len, _recv_buffer + copy_len, remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            _recv_msg_node->_msg[_recv_msg_node->_cur_len] = '\0';
            std::cout << "receive data is " << _recv_msg_node->_msg << endl;
            Json::Reader reader;
            Json::Value root;
            reader.parse(std::string(_recv_msg_node->_msg), root);
            std::cout << "received json data: " << root["id"].asInt()<< ", " 
                    << root["name"].asString() << ", "
                    << root["value"].asDouble() << std::endl;
            std::string return_str = root.toStyledString();
            Send(return_str.c_str(), return_str.size());
            //继续轮询剩余未处理数据
            _b_head_parsed = false;
            _recv_head_node->Clear();
            if (bytes_transferred <= 0) {
                ::memset(_recv_buffer, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_recv_buffer, MAX_LENGTH),
                    std::bind(&Session::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                return;
            }
            continue;
        }
    }else {
        std::cout << "handle read failed, error is " << error.what() << endl;
        _server->ClearSession(_uuid);
    }
}

void Session::HandleReadHead(const boost::system::error_code& error, 
    size_t bytes_transferred, shared_ptr<Session> _self_shared){
    if(!error){
        if(bytes_transferred < HEAD_TOTAL_LEN){
            cout << "read head length error";
            Close();
            _server->ClearSession(_uuid);
            return;
        }

        //头部收全，解析头部获取消息长度
        short data_len = 0;
        memcpy(&data_len, _recv_head_node->_msg, HEAD_TOTAL_LEN);
        cout << "data len is " << data_len << endl;
    
        //头部不合法的情况下
        if (data_len > MAX_LENGTH){
            // 消息长度超过最大限制，关闭会话
            cerr << "Message length exceeds maximum limit: " << data_len << endl;
            Close();
            _server->ClearSession(_uuid);
            return;
        }

        _recv_msg_node = make_shared<MsgNode>(data_len);
        boost::asio::async_read(_socket, boost::asio::buffer(_recv_msg_node->_msg, 
            _recv_msg_node->_total_len), std::bind(&Session::HandleRead, this, 
            placeholders::_1, placeholders::_2, _self_shared));
    }else{
        cerr << "Read head error: " << error.message() << endl;
        Close();
        _server->ClearSession(_uuid);
    }
}

void Session::HandleReadMsg(const boost::system::error_code& error, 
    size_t bytes_transferred, shared_ptr<Session> _self_shared){
    if(!error){
        //模拟处理数据的耗时操作
        //PrintRecvData(_recv_msg_node->_msg, bytes_transferred);
        //std::chrono::milliseconds dura(2000);
        //std::this_thread::sleep_for(dura);
        _recv_msg_node->_msg[_recv_msg_node->_cur_len] = '\0';
        std::cout << "Received data is " << _recv_msg_node->_msg << endl;
        //回显数据  
        Send(_recv_msg_node->_msg, _recv_msg_node->_cur_len);
        //准备读取下一个消息头
        _recv_head_node->Clear();
        boost::asio::async_read(_socket, boost::asio::buffer(_recv_head_node->_msg, HEAD_TOTAL_LEN),
            std::bind(&Session::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
    }
}

void Session::HandleWrite(const boost::system::error_code& error, 
    shared_ptr<Session> _self_shared){
    if(!error){
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_queue.pop();
        if(!_send_queue.empty()){
            auto &msgnode = _send_queue.front();
            // 继续发送队列中的下一条消息
            boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_msg, msgnode->_total_len),
                std::bind(&Session::HandleWrite, this, placeholders::_1, _self_shared));
        }
    }else{
        cerr << "Write error: " << error.message() << endl;
        _server->ClearSession(_uuid);
    }
}

// 打印接收到的数据的十六进制表示（用于粘包测试）
void Session::PrintRecvData(char* data, int length){
    stringstream ss;
    string result = "0x";
    for (int i = 0; i < length; i++){
        string hexstr;
        ss << hex << std::setw(2) << std::setfill('0') << (static_cast<int>(static_cast<unsigned char>(data[i])));
        ss >> hexstr;
        result += hexstr;  
    }
    std::cout << "Received data in hex: " << result << endl;
}
