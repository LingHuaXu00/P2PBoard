# 根目录：项目全局配置
project('clipboard-sync', 'cpp', default_options : ['cpp_std=c++17'])
boost_dep = dependency('boost', modules : ['system', 'asio', 'beast'])
subdir('server')  # 进入子目录
subdir('client')  # 进入子目录

# server/ 目录：专门编译服务器
executable('clipboard-server', 'server.cpp', dependencies : boost_dep)