set(CMAKE_TOOLCHAIN_FILE "~/vcpkg/scripts/buildsystems/vcpkg.cmake")
cmake_minimum_required(VERSION 3.10)
project(finger_cli)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找到 nlohmann_json
find_package(nlohmann_json REQUIRED)

# 添加可执行文件
add_executable(finger_cli
    main.cpp
    finger_device.cpp
)

# 链接依赖库
target_link_libraries(finger_cli
    PRIVATE
    nlohmann_json::nlohmann_json
    dl
)
