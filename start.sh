#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 检查是否以 root 权限运行
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}错误: 请使用 sudo 运行此脚本${NC}"
    exit 1
fi

# 检查必需的动态库
check_libraries() {
    local all_exists=true
    local libs=(
        "/lib/libzkfpcap.so"
        "/lib/libzkfp.so"
        "/lib/libzkfinger10.so"
    )

    echo -e "${YELLOW}检查动态库...${NC}"
    for lib in "${libs[@]}"; do
        if [ ! -f "$lib" ]; then
            echo -e "${RED}错误: 找不到库文件 $lib${NC}"
            all_exists=false
        else
            echo -e "${GREEN}找到库文件: $lib${NC}"
        fi
    done

    if [ "$all_exists" = false ]; then
        echo -e "${RED}错误: 缺少必需的库文件，请先安装所需的动态库${NC}"
        return 1
    fi

    return 0
}

# 检查服务是否已经运行
check_service_running() {
    if pgrep -f "finger_server" > /dev/null; then
        echo -e "${RED}错误: 指纹服务已经在运行${NC}"
        return 1
    fi
    return 0
}

# 检查可执行文件
check_executable() {
    if [ ! -f "./finger_server" ]; then
        echo -e "${RED}错误: 找不到 finger_server 可执行文件${NC}"
        return 1
    fi
    
    if [ ! -x "./finger_server" ]; then
        echo -e "${YELLOW}添加执行权限...${NC}"
        chmod +x ./finger_server
    fi
    
    return 0
}

# 更新库缓存
update_library_cache() {
    echo -e "${YELLOW}更新库缓存...${NC}"
    sudo ldconfig
    return 0
}

# 主函数
main() {
    echo -e "${YELLOW}启动指纹服务...${NC}"
    
    # 运行检查
    check_service_running || exit 1
    check_executable || exit 1
    check_libraries || exit 1
    update_library_cache || exit 1
    
    # 启动服务
    echo -e "${YELLOW}正在启动服务...${NC}"
    nohup ./finger_server > finger_server.log 2>&1 &
    
    # 等待服务启动
    sleep 2
    
    # 检查服务是否成功启动
    if pgrep -f "finger_server" > /dev/null; then
        echo -e "${GREEN}服务启动成功！${NC}"
        echo -e "${YELLOW}服务日志: finger_server.log${NC}"
        # 显示最新日志
        tail -f finger_server.log
    else
        echo -e "${RED}服务启动失败，请检查日志文件${NC}"
        exit 1
    fi
}

# 捕获 Ctrl+C
trap 'echo -e "\n${YELLOW}正在停止服务...${NC}"; pkill -f "finger_server"; exit 0' INT

# 运行主函数
main

# 保持脚本运行
echo -e "${YELLOW}按 Ctrl+C 停止服务${NC}"
while true; do
    sleep 1
done 
