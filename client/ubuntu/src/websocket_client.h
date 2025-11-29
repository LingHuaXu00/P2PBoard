#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <memory>
#include <string>
#include <thread>
#include <atomic>

// Boost库
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

// 前向声明
class ClipboardManager;

/**
 * @class WebSocketClient
 * @brief 管理与P2PBoard服务器的WebSocket连接
 *
 * 此类处理WebSocket连接的整个生命周期：
 * - 连接建立和维护
 * - 消息发送和接收
 * - 错误处理和优雅关闭
 *
 * 客户端连接到服务器，在剪贴板内容变化时发送内容，
 * 并从其他客户端接收剪贴板更新。
 */
class WebSocketClient {
public:
    /**
     * @brief 构造函数
     * 使用默认设置初始化WebSocket客户端
     */
    WebSocketClient();

    /**
     * @brief 析构函数
     * 清理资源并在连接时断开连接
     */
    ~WebSocketClient();

    /**
     * @brief 连接到服务器
     *
     * @param server_url 服务器URL，格式为ws://host:port
     * @return 连接成功返回true，否则返回false
     */
    bool connect(const std::string& server_url);

    /**
     * @brief 从服务器断开连接
     */
    void disconnect();

    /**
     * @brief 发送消息到服务器
     *
     * @param message 要发送的消息（通常是剪贴板内容）
     */
    void send_message(const std::string& message);

    /**
     * @brief 处理来自服务器的传入消息
     *
     * 此方法应定期调用以检查新消息。
     * 它读取任何可用消息并调用handle_received_message。
     */
    void process_messages();

private:
    /**
     * @brief 启动读取线程
     *
     * 此方法启动后台线程，持续检查来自服务器的传入消息。
     */
    void start_reading_thread();

    /**
     * @brief 处理来自服务器的消息
     *
     * 此方法处理从服务器接收的消息。在此实现中，
     * 它只是打印消息，但在实际应用中它会根据消息内容
     * 更新本地剪贴板或触发其他操作。
     *
     * @param message 接收的消息
     */
    void handle_received_message(const std::string& message);

    // Boost ASIO上下文用于I/O操作
    std::unique_ptr<boost::asio::io_context> context_;

    // DNS查找解析器
    std::unique_ptr<boost::asio::ip::tcp::resolver> resolver_;

    // WebSocket流
    std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_;

    // 连接状态
    std::atomic<bool> connected_;
    std::atomic<bool> stopped_;

    // 读取线程
    std::thread reading_thread_;
};

#endif // WEBSOCKET_CLIENT_H