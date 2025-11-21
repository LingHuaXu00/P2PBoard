#include "server.h"
#include <iostream>

int main()
{
    try
    {
        net::io_context ioc;
        session_manager manager;

        clipboard_server server(ioc,
                                tcp::endpoint{tcp::v4(), 8080},
                                manager);

        std::cout << "剪贴板同步服务器启动在端口 8080\n";
        ioc.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "错误: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
