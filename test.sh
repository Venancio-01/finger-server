#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试结果统计
TOTAL=0
PASSED=0

# 测试函数
run_test() {
    local name=$1
    local command=$2
    local expected_success=$3
    
    echo -e "\n${YELLOW}Running test: ${name}${NC}"
    echo "Command: $command"
    
    # 使用 sudo 发送命令到 finger_cli 并获取响应
    response=$(echo $command | sudo ./finger_cli)
    echo "Response: $response"
    
    # 检查响应中的 success 字段
    success=$(echo $response | grep -o '"success":\s*\(true\|false\)')
    
    TOTAL=$((TOTAL + 1))
    
    if [[ "$success" == *"$expected_success"* ]]; then
        echo -e "${GREEN}✓ Test passed${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗ Test failed${NC}"
        echo "Expected success: $expected_success"
        echo "Got: $success"
    fi
}

# 确保在 build 目录下运行
if [ ! -f "./finger_cli" ]; then
    echo -e "${RED}Error: finger_cli not found in current directory${NC}"
    echo "Please run this script from the build directory"
    exit 1
fi

# 检查是否有 sudo 权限
if ! sudo -v; then
    echo -e "${RED}Error: This script requires sudo privileges${NC}"
    exit 1
fi

echo -e "${YELLOW}Starting finger_cli tests with sudo...${NC}\n"

# 测试 SDK 初始化
run_test "Initialize SDK" \
    '{"cmd": "initialize"}' \
    "true"

sleep 1

# 测试设备连接状态
run_test "Check device connection" \
    '{"cmd": "isConnected"}' \
    "true"

sleep 1

# 测试打开设备
run_test "Open device" \
    '{"cmd": "openDevice"}' \
    "true"

sleep 1

# 测试采集指纹
run_test "Capture fingerprint" \
    '{"cmd": "capture"}' \
    "true"

sleep 1

# 测试关闭设备
run_test "Close device" \
    '{"cmd": "closeDevice"}' \
    "true"

sleep 1

# 测试销毁 SDK
run_test "Uninitialize SDK" \
    '{"cmd": "uninitialize"}' \
    "true"

# 打印测试结果统计
echo -e "\n${YELLOW}Test Summary${NC}"
echo "Total tests: $TOTAL"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$((TOTAL - PASSED))${NC}"

# 根据测试结果设置退出码
if [ $PASSED -eq $TOTAL ]; then
    echo -e "\n${GREEN}✓ All tests passed${NC}"
    exit 0
else
    echo -e "\n${RED}✗ Some tests failed${NC}"
    exit 1
fi
