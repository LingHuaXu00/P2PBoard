#include "server.h"
#include <iostream>

void session_manager::add(websocket::stream<tcp::socket> *ws)
{
    sessions_.insert(ws);
    std::cout << "新设备连接，当前连接数: " << sessions_.size() << "\n";
}

void session_manager::remove(websocket::stream<tcp::socket> *ws)
{
    sessions_.erase(ws);
    std::cout << "设备断开，当前连接数: " << sessions_.size() << "\n";
}

void session_manager::broadcast(const std::string &message)
{
    std::cout << "广播剪贴板内容，长度: " << message.length() << "\n";

    for (auto *ws : sessions_)
    {
        ws->async_write(net::buffer(message),
                        [](beast::error_code ec, std::size_t)
                        {
                            if (ec)
                            {
                                std::cerr << "发送失败: " << ec.message() << "\n";
                            }
                        });
    }
}

clipboard_server::clipboard_server(net::io_context &ioc, tcp::endpoint endpoint, session_manager &manager)
    : ioc_(ioc), acceptor_(ioc, endpoint), manager_(manager)
{
    do_accept();
}

void clipboard_server::do_accept()
{
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
                do_handshake(ws);
            }
            do_accept();
        });
}

void clipboard_server::do_handshake(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
    ws->async_accept(
        [this, ws](beast::error_code ec)
        {
            if (!ec)
            {
                manager_.add(ws.get());
                do_read(ws);
            }
        });
}

void clipboard_server::do_read(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
    auto buffer = std::make_shared<beast::flat_buffer>();

    ws->async_read(*buffer,
                   [this, ws, buffer](beast::error_code ec, std::size_t)
                   {
                       if (!ec)
                       {
                           std::string message = beast::buffers_to_string(buffer->data());
                           manager_.broadcast(message);
                           do_read(ws);
                       }
                       else
                       {
                           manager_.remove(ws.get());
                       }
                   });
}
