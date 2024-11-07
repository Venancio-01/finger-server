#include <iostream>
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

// 颜色定义
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RESET   "\033[0m"

// 测试用例执行函数
std::string exec_command(FILE* pipe, const std::string& command) {
    fprintf(pipe, "%s\n", command.c_str());
    fflush(pipe);
    
    char buffer[1024];
    std::string result;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result = buffer;
    }
    return result;
}

// 运行测试用例
void run_test(FILE* pipe, const std::string& name, const std::string& command) {
    std::cout << YELLOW << "\nRunning test: " << name << RESET << std::endl;
    std::cout << "Command: " << command << std::endl;
    
    std::string response = exec_command(pipe, command);
    std::cout << "Response: " << response;
    
    // 简单的成功判断，根据实际情况可以修改
    if (response.find("\"success\":true") != std::string::npos) {
        std::cout << GREEN << "✓ Test passed" << RESET << std::endl;
    } else {
        std::cout << RED << "✗ Test failed" << RESET << std::endl;
    }
    
    // 在测试之间添加短暂延时
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    std::cout << YELLOW << "Starting finger_cli tests..." << RESET << std::endl;
    
    // 使用 popen 启动 finger_cli 进程
    FILE* pipe = popen("sudo ./finger_cli", "r+");
    if (!pipe) {
        std::cerr << RED << "Failed to start finger_cli" << RESET << std::endl;
        return 1;
    }
    
    try {
        // 测试 SDK 初始化
        run_test(pipe, "Initialize SDK", 
                R"({"cmd": "initialize"})");
        
        // 测试设备连接状态
        run_test(pipe, "Check device connection", 
                R"({"cmd": "isConnected"})");
        
        // 测试打开设备
        run_test(pipe, "Open device", 
                R"({"cmd": "openDevice"})");
        
        // 测试采集指纹
        run_test(pipe, "Capture fingerprint", 
                R"({"cmd": "capture"})");
        
        // 测试关闭设备
        run_test(pipe, "Close device", 
                R"({"cmd": "closeDevice"})");
        
        // 测试销毁 SDK
        run_test(pipe, "Uninitialize SDK", 
                R"({"cmd": "uninitialize"})");
        
    } catch (const std::exception& e) {
        std::cerr << RED << "Test failed with error: " << e.what() << RESET << std::endl;
    }
    
    // 关闭管道
    pclose(pipe);
    
    std::cout << YELLOW << "\nTests completed" << RESET << std::endl;
    return 0;
} 
