cmake_minimum_required(VERSION 3.10) # 调低版本要求以增加兼容性

project(fix_gg)

# 移除硬编码的 D:/ 路径，让外部环境（如 GitHub Actions）决定 NDK 路径
if (DEFINED CMAKE_ANDROID_NDK)
    set(ANDROID_NDK ${CMAKE_ANDROID_NDK})
endif ()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含当前目录，确保能找到 "dev增强版对接.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 编译动态库
add_library(fix_gg SHARED library.cpp)

# 链接安卓日志库
target_link_libraries(fix_gg
    log
)
