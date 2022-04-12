## TIT Coroutine Lib

本仓库是模仿 cocoyaxi 库所做的学习用仓库，原仓库见[cocoyaxi](https://github.com/idealvin/cocoyaxi)

主要关注于其协程调度模块，其他的模块如日志、线程、命令参数解析则略过

目前实现效果为，能够正常地调度普通协程任务，暂不能调度 fd 相关的IO任务

因为使用到了自己写的日志库与基本部件库，编译前需要将third_party目录下的两个静态库安装到 /usr/lib下，
也可以直接执行 third_party 目录下的 install.sh，将会自动安装那两个静态库
