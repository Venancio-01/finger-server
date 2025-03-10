cmake_minimum_required(VERSION 3.10)

# 设置 vcpkg 工具链文件
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(finger_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找到 nlohmann_json
find_package(nlohmann_json REQUIRED)

# 添加 pthread 库
find_package(Threads REQUIRED)

# 添加 cpprestsdk 依赖
find_package(cpprestsdk REQUIRED)

add_executable(finger_server
    main.cpp
    finger_device.cpp
    finger_algorithm.cpp
    base64.cpp
)

target_link_libraries(finger_server
    PRIVATE
    cpprestsdk::cpprest
    nlohmann_json::nlohmann_json
    dl
)
