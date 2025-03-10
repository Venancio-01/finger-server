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

# 添加头文件目录
include_directories(${CMAKE_SOURCE_DIR}/demo/include)

add_executable(finger_server
    main.cpp
    base64.cpp
    logger.cpp
    zkfp_sdk.cpp
)

target_link_libraries(finger_server
    PRIVATE
    cpprestsdk::cpprest
    nlohmann_json::nlohmann_json
    dl
    zkfp
    zkfpcap
    zkalg12
    zkfinger10
    zksensorcore
    idfprcap
    silkidcap
    usb-1.0
    Threads::Threads
)
