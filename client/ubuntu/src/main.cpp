#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

// 包含我们的头文件
#include "websocket_client.h"
#include "clipboard_manager.h"
#include "config.h"

// 全局标志位用于控制应用程序循环
volatile bool running = true;

// 信号处理器用于优雅关闭
void signal_handler(int sig) {
    std::cout << "\n收到信号 " << sig << "。正在关闭..." << std::endl;
    running = false;
}

int main() {
    // 设置信号处理器用于优雅关闭
    #ifdef _POSIX_C_SOURCE
        #include <signal.h>
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    #endif

    std::cout << "启动P2PBoard Ubuntu客户端..." << std::endl;

    // 初始化剪贴板管理器
    ClipboardManager clipboard_manager;
    if (!clipboard_manager.initialize()) {
        std::cerr << "初始化剪贴板管理器失败。退出。" << std::endl;
        return 1;
    }

    // 初始化WebSocket客户端
    WebSocketClient websocket_client;
    std::string server_url = std::string(SERVER_PROTOCOL) + SERVER_HOST + ":" + SERVER_PORT;

    std::cout << "连接到服务器 " << server_url << std::endl;

    if (!websocket_client.connect(server_url)) {
        std::cerr << "连接服务器失败。退出。" << std::endl;
        return 1;
    }

    // 启动剪贴板监控线程
    std::thread clipboard_thread([&]() {
        while (running) {
            // 获取当前剪贴板内容
            std::string clipboard_content = clipboard_manager.get_clipboard_content();

            // 如果有新的内容则发送到服务器
            if (!clipboard_content.empty()) {
                websocket_client.send_message(clipboard_content);
            }

            // 等待同步间隔
            std::this_thread::sleep_for(std::chrono::milliseconds(CLIPBOARD_SYNC_INTERVAL));
        }
    });

    // 主循环 - 保持连接活跃
    while (running) {
        // 检查来自服务器的传入消息
        websocket_client.process_messages();

        // 小延迟以防止忙等待
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 清理资源
    std::cout << "正在关闭客户端..." << std::endl;
    websocket_client.disconnect();

    // 等待剪贴板线程结束
    if (clipboard_thread.joinable()) {
        clipboard_thread.join();
    }

    std::cout << "客户端关闭完成。" << std::endl;
    return 0;
}