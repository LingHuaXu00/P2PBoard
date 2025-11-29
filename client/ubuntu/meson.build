# Meson build file for P2PBoard Ubuntu Client
# Author: Claude Code
# Date: 2025-11-29

project('p2pboard_client', 'cpp',
    version : '1.0.0',
    default_options : ['warning_level=3', 'werror=true'])

# Find required dependencies
boost_dep = dependency('boost', version : '>=1.83.0',
    required : true,
    modules : ['system'])

openssl_dep = dependency('openssl', version : '>=1.1.0', required : true)
wayland_dep = dependency('wayland-client', version : '>=1.18.0', required : true)

# Create the executable
executable = executable('p2pboard_client',
    'src/main.cpp',
    'src/clipboard_manager.cpp',
    'src/websocket_client.cpp',
    dependencies : [boost_dep, openssl_dep, wayland_dep],
    cpp_args : ['-Wall', '-Wextra', '-Wpedantic'])

# Install the executable
install(executable, install_dir: '/usr/local/bin')
