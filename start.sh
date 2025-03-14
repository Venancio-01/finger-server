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

# 获取当前登录用户
CURRENT_USER=$(who | awk '{print $1}' | head -n1)

# 检查必需的动态库
check_libraries() {
    local all_exists=true
    local libs=(
        "/usr/local/lib/libzkfpcap.so"
        "/usr/local/lib/libzkfp.so"
        "/usr/local/lib/libzkfinger10.so"
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
check_services_running() {
    if pgrep -f "finger_server" > /dev/null; then
        echo -e "${RED}错误: 指纹服务已经在运行${NC}"
        return 1
    fi
    if pgrep -f "smart-cabinet" > /dev/null; then
        echo -e "${RED}错误: smart-cabinet 已经在运行${NC}"
        return 1
    fi
    return 0
}

# 检查可执行文件
check_executables() {
    if [ ! -f "./finger_server" ]; then
        echo -e "${RED}错误: 找不到 finger_server 可执行文件${NC}"
        return 1
    fi
    
    if [ ! -f "/opt/smart-cabinet/@smart-cabinetsmart-cabinet" ]; then
        echo -e "${RED}错误: 找不到 smart-cabinet 可执行文件${NC}"
        return 1
    fi
    
    if [ ! -x "./finger_server" ]; then
        echo -e "${YELLOW}添加 finger_server 执行权限...${NC}"
        chmod +x ./finger_server
    fi
    
    return 0
}

# 更新动态库缓存
update_library_cache() {
    echo -e "${YELLOW}更新动态库缓存...${NC}"
    ldconfig
    if [ $? -ne 0 ]; then
        echo -e "${RED}错误: 更新动态库缓存失败${NC}"
        return 1
    fi
    return 0
}

# 启动服务
start_services() {
    # 启动指纹服务
    echo -e "${YELLOW}正在启动指纹服务...${NC}"
    nohup ./finger_server > finger_server.log 2>&1 &
    FINGER_PID=$!
    
    # 等待指纹服务启动
    sleep 2
    
    if ! ps -p $FINGER_PID > /dev/null; then
        echo -e "${RED}指纹服务启动失败${NC}"
        return 1
    fi
    echo -e "${GREEN}指纹服务启动成功！${NC}"
    
    # 启动 smart-cabinet
    echo -e "${YELLOW}正在启动 smart-cabinet...${NC}"
    
    # 设置 Electron 应用需要的环境变量
    export DISPLAY=:0
    export XAUTHORITY="/home/$CURRENT_USER/.Xauthority"
    export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$(id -u $CURRENT_USER)/bus"
    export XDG_RUNTIME_DIR="/run/user/$(id -u $CURRENT_USER)"
    export NO_AT_BRIDGE=1
    export ELECTRON_NO_SANDBOX=1
    
    # 使用 su 切换到当前用户来运行 Electron 应用
    su - $CURRENT_USER -c "cd /opt/smart-cabinet && \
        DISPLAY=:0 \
        ELECTRON_NO_SANDBOX=1 \
        NO_AT_BRIDGE=1 \
        XDG_RUNTIME_DIR=/run/user/$(id -u $CURRENT_USER) \
        DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u $CURRENT_USER)/bus \
        nohup ./@smart-cabinetsmart-cabinet > /tmp/smart_cabinet.log 2>&1 &"
    
    # 等待 Electron 应用启动
    sleep 5
    
    if ! pgrep -f "smart-cabinet" > /dev/null; then
        echo -e "${RED}smart-cabinet 启动失败${NC}"
        kill $FINGER_PID
        return 1
    fi
    echo -e "${GREEN}smart-cabinet 启动成功！${NC}"
    
    return 0
}

# 停止服务
stop_services() {
    echo -e "\n${YELLOW}正在停止服务...${NC}"
    pkill -f "finger_server"
    su - $CURRENT_USER -c "pkill -f smart-cabinet"
    echo -e "${GREEN}服务已停止${NC}"
}

# 主函数
main() {
    echo -e "${YELLOW}启动服务...${NC}"
    
    # 运行检查
    check_services_running || exit 1
    check_executables || exit 1
    check_libraries || exit 1
    update_library_cache || exit 1
    
    # 启动服务
    if ! start_services; then
        echo -e "${RED}服务启动失败${NC}"
        exit 1
    fi
    
    # 显示日志
    echo -e "${YELLOW}服务日志:${NC}"
    echo -e "指纹服务日志: finger_server.log"
    echo -e "smart-cabinet 日志: /tmp/smart_cabinet.log"
    
    # 使用 tail -f 同时监控两个日志文件
    tail -f finger_server.log /tmp/smart_cabinet.log
}

# 捕获 Ctrl+C
trap 'stop_services; exit 0' INT TERM

# 运行主函数
main
