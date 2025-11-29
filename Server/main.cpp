#include "server.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
    try
    {
        // 默认端口8080，可通过命令行参数修改
        unsigned short port = 8080;
        if (argc >= 2)
            port = static_cast<unsigned short>(std::atoi(argv[1]));

        // 创建IO上下文
        net::io_context ioc;
        // 创建会话管理器
        session_manager manager;

        // 创建剪贴板服务器，监听指定端口
        clipboard_server server(ioc,
                                tcp::endpoint{tcp::v4(), port},
                                manager);

        std::cout << "剪贴板同步服务器启动在端口 " << port << "\n";
        // 运行IO上下文事件循环
        ioc.run();
    }
    catch (std::exception &e)
    {
        // 异常处理
        std::cerr << "错误: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
