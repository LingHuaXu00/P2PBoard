#ifndef P2PBOARD_CONFIG_H
#define P2PBOARD_CONFIG_H

// 服务器配置
#define SERVER_HOST "localhost"
#define SERVER_PORT "8080"
#define SERVER_PROTOCOL "ws://"

// 连接超时时间(秒)
#define CONNECTION_TIMEOUT 30

// 剪贴板同步间隔(毫秒)
#define CLIPBOARD_SYNC_INTERVAL 1000

// 最大消息大小(字节)
#define MAX_MESSAGE_SIZE 1024 * 1024  // 1MB

// 调试日志标志
#define DEBUG_LOGGING 1

#endif // P2PBOARD_CONFIG_H