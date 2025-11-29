# 根目录：项目全局配置
project('clipboard-sync', 'cpp', default_options : ['cpp_std=c++17'])

# Boost依赖 - asio和beast是header-only库，只需要包含路径
boost_dep = dependency('boost', modules : ['system'])

subdir('Server')  # 进入子目录
subdir('client')  # 进入子目录
