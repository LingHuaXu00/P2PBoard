#include "websocket_client.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>

// 构造函数
WebSocketClient::WebSocketClient() {
    // 初始化上下文和解析器
    context_ = std::make_unique<boost::asio::io_context>();
    resolver_ = std::make_unique<boost::asio::ip::tcp::resolver>(*context_);

    // 初始化流
    ws_ = std::make_unique<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(*context_);

    // 初始化连接状态
    connected_ = false;
    stopped_ = false;
}

// 析构函数
WebSocketClient::~WebSocketClient() {
    if (connected_) {
        disconnect();
    }
}

// 连接到服务器
bool WebSocketClient::connect(const std::string& server_url) {
    try {
        // 解析服务器URL
        std::size_t protocol_end = server_url.find("://");
        std::size_t host_start = protocol_end + 3;
        std::size_t port_start = server_url.find(":", host_start);
        std::size_t host_end = server_url.find("/", host_start);

        if (protocol_end == std::string::npos || host_start >= server_url.size()) {
            std::cerr << "无效的服务器URL格式: " << server_url << std::endl;
            return false;
        }

        std::string protocol = server_url.substr(0, protocol_end);
        std::string host = server_url.substr(host_start, (port_start != std::string::npos ? port_start : host_end) - host_start);
        std::string port = (port_start != std::string::npos && host_end != std::string::npos) ?
                            server_url.substr(port_start + 1, host_end - port_start - 1) : "80";

        // 解析主机
        auto const results = resolver_->resolve(host, port);

        // 创建套接字
        boost::asio::ip::tcp::socket socket(*context_);

        // 连接到服务器
        boost::asio::connect(socket, results->begin(), results->end());

        // 设置WebSocket流
        ws_->initiate(std::move(socket));

        // 执行WebSocket握手
        boost::beast::websocket::request_type req;
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, "P2PBoard-Client/1.0");
        req.set(boost::beast::http::field::connection, "Upgrade");
        req.set(boost::beast::http::field::upgrade, "websocket");

        // 执行握手
        boost::beast::error_code ec;
        ws_->handshake(req, ec);

        if (ec) {
            std::cerr << "WebSocket握手失败: " << ec.message() << std::endl;
            return false;
        }

        // 成功连接
        connected_ = true;
        std::cout << "成功连接到服务器 " << server_url << std::endl;

        // 在单独的线程中启动读取消息
        start_reading_thread();

        return true;

    } catch (const std::exception& e) {
        std::cerr << "连接过程中发生异常: " << e.what() << std::endl;
        return false;
    }
}

// 从服务器断开连接
void WebSocketClient::disconnect() {
    if (!connected_) {
        return;
    }

    try {
        // 关闭WebSocket连接
        boost::beast::error_code ec;
        ws_->close(boost::beast::websocket::close_reason::normal, ec);

        if (ec) {
            std::cerr << "关闭WebSocket连接时出错: " << ec.message() << std::endl;
        }

        // 停止读取线程
        stopped_ = true;
        if (reading_thread_.joinable()) {
            reading_thread_.join();
        }

        connected_ = false;
        std::cout << "已从服务器断开连接。" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "断开连接过程中发生异常: " << e.what() << std::endl;
    }
}

// 发送消息到服务器
void WebSocketClient::send_message(const std::string& message) {
    if (!connected_) {
        std::cerr << "无法发送消息: 未连接到服务器" << std::endl;
        return;
    }

    try {
        // 检查消息大小
        if (message.length() > MAX_MESSAGE_SIZE) {
            std::cerr << "消息过大 (" << message.length() << " 字节)。最大允许值: " << MAX_MESSAGE_SIZE << std::endl;
            return;
        }

        // 发送消息
        boost::beast::error_code ec;
        boost::beast::flat_buffer buffer;
        boost::beast::ostream(buffer) << message;

        ws_->write(buffer.data(), ec);

        if (ec) {
            std::cerr << "发送消息时出错: " << ec.message() << std::endl;
            connected_ = false;  // 出错时标记为断开连接
        }

    } catch (const std::exception& e) {
        std::cerr << "发送消息时发生异常: " << e.what() << std::endl;
        connected_ = false;  // 出错时标记为断开连接
    }
}

// 处理传入消息
void WebSocketClient::process_messages() {
    if (!connected_) {
        return;
    }

    try {
        // 读取任何可用消息
        boost::beast::error_code ec;
        boost::beast::flat_buffer buffer;

        // 读取消息
        ws_->read(buffer, ec);

        if (ec) {
            if (ec != boost::beast::websocket::error::closed) {
                std::cerr << "读取消息时出错: " << ec.message() << std::endl;
            }
            connected_ = false;  // 出错时标记为断开连接
            return;
        }

        // 提取消息内容
        std::string message = boost::beast::buffers_to_string(buffer.data());

        // 处理接收到的消息
        handle_received_message(message);

    } catch (const std::exception& e) {
        std::cerr << "处理消息时发生异常: " << e.what() << std::endl;
        connected_ = false;  // 出错时标记为断开连接
    }
}

// 启动读取线程
void WebSocketClient::start_reading_thread() {
    stopped_ = false;
    reading_thread_ = std::thread([this]() {
        while (!stopped_) {
            // 检查传入消息
            process_messages();

            // 小延迟以防止忙等待
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
}

// 处理接收到的消息
void WebSocketClient::handle_received_message(const std::string& message) {
    // 简单实现：仅打印消息
    std::cout << "从服务器接收到: " << message << std::endl;

    // 在实际实现中，您应该:
    // 1. 验证消息格式
    // 2. 如果是剪贴板更新则更新本地剪贴板
    // 3. 根据消息内容触发适当的操作

    // 目前假设所有消息都是剪贴板内容
    // 这将由剪贴板管理器处理
}