project('clipboard-server', 'cpp',
  version : '1.0',
  default_options : ['cpp_std=c++17'])

boost_dep = dependency('boost', modules: ['system'])

executable('clipboard-server',
           'main.cpp',
           'server.cpp',
           dependencies : boost_dep,
           install : true)