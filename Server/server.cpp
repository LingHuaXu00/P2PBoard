#include "server.h"
#include <iostream>

// 添加新的WebSocket会话到管理器
void session_manager::add(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
    // 使用锁保护共享数据
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    // 插入原始指针以避免循环引用
    sessions_.insert(ws.get());
    std::cout << "新设备连接，当前连接数: " << sessions_.size() << "\n";
}

// 从管理器中移除WebSocket会话
void session_manager::remove(websocket::stream<tcp::socket> *ws)
{
    // 使用锁保护共享数据
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(ws);
    std::cout << "设备断开，当前连接数: " << sessions_.size() << "\n";
}

// 向所有连接的客户端广播消息
void session_manager::broadcast(const std::string &message)
{
    // 验证消息有效性
    if (message.empty())
    {
        std::cout << "忽略空消息广播\n";
        return;
    }

    // 限制消息最大长度
    const size_t MAX_MESSAGE_SIZE = 1024 * 1024; // 1MB
    if (message.length() > MAX_MESSAGE_SIZE)
    {
        std::cerr << "消息过大，拒绝广播\n";
        return;
    }

    std::cout << "广播剪贴板内容，长度: " << message.length() << "\n";

    // 使用锁保护共享数据
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    // 遍历所有会话并发送消息
    for (auto *ws : sessions_)
    {
        // 异步写入数据到WebSocket
        ws->async_write(net::buffer(message),
                        [ws](beast::error_code ec, std::size_t)
                        {
                            // 错误处理
                            if (ec)
                            {
                                std::cerr << "发送失败: " << ec.message()
                                          << " (code: " << ec.value() << ")" << std::endl;
                            }
                        });
    }
}

// 剪贴板服务器构造函数
clipboard_server::clipboard_server(net::io_context &ioc, tcp::endpoint endpoint, session_manager &manager)
    : ioc_(ioc), acceptor_(ioc, endpoint), manager_(manager)
{
    // 开始接受连接
    do_accept();
}

// 异步接受新连接
void clipboard_server::do_accept()
{
    // 异步接受连接
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket)
        {
            // 如果没有错误，处理新连接
            if (!ec)
            {
                // 创建WebSocket流
                auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
                // 进行WebSocket握手
                do_handshake(ws);
            }
            // 继续接受下一个连接
            do_accept();
        });
}

// 处理WebSocket握手
void clipboard_server::do_handshake(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
    // 异步接受WebSocket连接
    ws->async_accept(
        [this, ws](beast::error_code ec)
        {
            // 如果握手成功
            if (!ec)
            {
                // 添加会话到管理器
                manager_.add(ws);
                // 开始读取数据
                do_read(ws);
            }
        });
}

// 从客户端读取数据
void clipboard_server::do_read(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
    // 创建缓冲区存储接收到的数据
    auto buffer = std::make_shared<beast::flat_buffer>();

    // 异步读取数据
    ws->async_read(*buffer,
                   [this, ws, buffer](beast::error_code ec, std::size_t)
                   {
                       // 如果读取成功
                       if (!ec)
                       {
                           // 将缓冲区数据转换为字符串
                           std::string message = beast::buffers_to_string(buffer->data());
                           // 广播消息给所有客户端
                           manager_.broadcast(message);
                           // 继续读取下一个消息
                           do_read(ws);
                       }
                       else
                       {
                           // 如果读取出错，移除会话
                           manager_.remove(ws.get());
                       }
                   });
}
