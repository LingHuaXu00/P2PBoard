#ifndef CLIPBOARD_SERVER_H
#define CLIPBOARD_SERVER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <unordered_set>
#include <mutex>

// 命名空间简写，提高代码可读性
namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;
namespace websocket = beast::websocket;

// 会话管理器类，负责管理所有WebSocket连接
class session_manager
{
public:
    // 添加新的WebSocket会话
    void add(std::shared_ptr<websocket::stream<tcp::socket>> ws);
    // 移除WebSocket会话
    void remove(websocket::stream<tcp::socket> *ws);
    // 向所有连接的客户端广播消息
    void broadcast(const std::string &message);

private:
    // 存储所有活跃的WebSocket会话
    std::unordered_set<websocket::stream<tcp::socket> *> sessions_;
    // 互斥锁，保证线程安全
    std::mutex sessions_mutex_;
};

// 剪贴板服务器类，处理WebSocket连接和消息
class clipboard_server
{
public:
    // 构造函数，初始化服务器并开始接受连接
    clipboard_server(net::io_context &ioc, tcp::endpoint endpoint, session_manager &manager);

private:
    // 异步接受新连接
    void do_accept();
    // 处理WebSocket握手
    void do_handshake(std::shared_ptr<websocket::stream<tcp::socket>> ws);
    // 从客户端读取数据
    void do_read(std::shared_ptr<websocket::stream<tcp::socket>> ws);

    // 引用IO上下文，用于异步操作
    net::io_context &ioc_;
    // TCP接受器，用于监听和接受连接
    tcp::acceptor acceptor_;
    // 引用会话管理器
    session_manager &manager_;
};

#endif
