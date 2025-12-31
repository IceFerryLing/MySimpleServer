#include "AsyncClient.h"

AsyncClient::AsyncClient(boost::asio::io_context& ioc, const string& ip, int port)
    : _socket(ioc), _endpoint(make_address(ip), port) {
    do_connect();
}

void AsyncClient::Close() {
    boost::asio::post(_socket.get_executor(), [this]() {
        _socket.close();
    });
}

void AsyncClient::Send(const string& msg) {
    boost::asio::post(_socket.get_executor(), [this, msg]() {
        bool write_in_progress = !_send_queue.empty();
        
        size_t request_length = msg.length();
        if (request_length > MAX_LENGTH - HEAD_LENGTH) {
            cout << "Message too long" << endl;
            return;
        }

        vector<char> send_data(request_length + HEAD_LENGTH);
        short len_short = static_cast<short>(request_length);
        
        memcpy(send_data.data(), &len_short, HEAD_LENGTH);
        memcpy(send_data.data() + HEAD_LENGTH, msg.c_str(), request_length);

        _send_queue.push(send_data);

        if (!write_in_progress) {
            do_write();
        }
    });
}

void AsyncClient::do_connect() {
    _socket.async_connect(_endpoint,
        [this](boost::system::error_code ec) {
            if (!ec) {
                cout << "Connected to server successfully." << endl;
                do_read_header();
            } else {
                cout << "connect failed, code is " << ec.value() << " error msg is " << ec.message() << endl;
            }
        });
}

void AsyncClient::do_read_header() {
    boost::asio::async_read(_socket,
        boost::asio::buffer(_recv_head, HEAD_LENGTH),
        [this](boost::system::error_code ec, size_t /*length*/) {
            if (!ec) {
                short msglen = 0;
                memcpy(&msglen, _recv_head, HEAD_LENGTH);
                do_read_body(msglen);
            } else {
                cout << "Read header failed: " << ec.message() << endl;
                Close();
            }
        });
}

void AsyncClient::do_read_body(short msglen) {
    _recv_msg.resize(msglen);
    boost::asio::async_read(_socket,
        boost::asio::buffer(_recv_msg, msglen),
        [this, msglen](boost::system::error_code ec, size_t /*length*/) {
            if (!ec) {
                cout << "Reply is: ";
                cout.write(_recv_msg.data(), msglen);
                cout << endl;
                cout << "Reply len is " << msglen << endl;
                do_read_header();
            } else {
                cout << "Read body failed: " << ec.message() << endl;
                Close();
            }
        });
}

void AsyncClient::do_write() {
    auto& data = _send_queue.front();
    boost::asio::async_write(_socket,
        boost::asio::buffer(data),
        [this](boost::system::error_code ec, size_t /*length*/) {
            if (!ec) {
                _send_queue.pop();
                if (!_send_queue.empty()) {
                    do_write();
                }
            } else {
                cout << "Write failed: " << ec.message() << endl;
                Close();
            }
        });
}
