#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 启动 finger-cli 程序
echo -e "${YELLOW}启动指纹识别服务...${NC}"
sudo $(which npx) ts-node src/index.ts > output.log 2>&1 &
CLI_PID=$!

# 等待程序启动
sleep 1

# 测试函数
send_command() {
    local name=$1
    local command=$2
    
    echo -e "\n${YELLOW}执行测试: ${name}${NC}"
    echo "发送命令: $command"
    
    # 发送命令并获取响应
    echo $command > /proc/$CLI_PID/fd/0
    sleep 0.5
    
    # 获取最后一行输出作为响应
    response=$(tail -n 1 output.log)
    echo "收到响应: $response"
    
    # 检查响应中的 success 字段
    if [[ $response == *"\"success\":true"* ]]; then
        echo -e "${GREEN}✓ 测试通过${NC}"
    else
        echo -e "${RED}✗ 测试失败${NC}"
    fi
}

# 执行测试用例
send_command "初始化 SDK" \
    '{"cmd": "init"}'

sleep 1

send_command "检查设备连接" \
    '{"cmd": "isConnected"}'

sleep 1

send_command "打开设备" \
    '{"cmd": "openDevice"}'

sleep 1

send_command "注册指纹-第一次" \
    '{"cmd": "register", "index": 0}'

echo -e "\n${YELLOW}请按压手指...${NC}"
sleep 3

send_command "注册指纹-第二次" \
    '{"cmd": "register", "index": 1}'

echo -e "\n${YELLOW}请再次按压同一个手指...${NC}"
sleep 3

send_command "注册指纹-第三次" \
    '{"cmd": "register", "index": 2}'

sleep 1

send_command "识别指纹" \
    '{"cmd": "identify"}'

echo -e "\n${YELLOW}请按压手指进行识别...${NC}"
sleep 3

send_command "关闭设备" \
    '{"cmd": "closeDevice"}'

sleep 1

send_command "销毁 SDK" \
    '{"cmd": "destroy"}'

# 清理
echo -e "\n${YELLOW}清理进程...${NC}"
kill $CLI_PID
rm output.log

echo -e "\n${GREEN}测试完成${NC}" 
