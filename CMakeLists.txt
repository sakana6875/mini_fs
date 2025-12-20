# 指定 CMake 最低版本 
cmake_minimum_required(VERSION 3.10)

# 项目名+使用的语言
project(mini_fs C)

# 指定 C 语言
set(CMAKE_C_STANDARD 17)

# 头文件搜索路径
include_directories(include)

# 添加可执行程序
add_executable(mini_fs
    src/fs.c
    src/main.c
    src/dir.c
    src/path.c
    src/file.c
    src/shell.c
)