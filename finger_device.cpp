#include "finger_device.h"
#include <dlfcn.h>
#include <iostream>

// SDK 函数指针定义
typedef int (*SensorEnumDevices)(DeviceInfo*, int);
typedef void* (*SensorOpen)(DeviceInfo*);
typedef int (*SensorClose)(void*);
typedef int (*SensorCapture)(void*, unsigned char*, int);
typedef int (*SensorGetParameter)(void*, int);
typedef int (*SensorStatus)(void*);  // 添加设备状态检查函数

static void* g_deviceSDK = nullptr;
static SensorEnumDevices g_enumDevices = nullptr;
static SensorOpen g_openDevice = nullptr;
static SensorClose g_closeDevice = nullptr;
static SensorCapture g_capture = nullptr;
static SensorGetParameter g_getParameter = nullptr;
static SensorStatus g_status = nullptr;  // 设备状态检查函数指针

bool FingerDevice::initSDK() {
    g_deviceSDK = dlopen("libzkfpcap.so", RTLD_LAZY);
    if (!g_deviceSDK) {
        std::cout << "加载设备 SDK 失败: " << dlerror() << std::endl;
        return false;
    }
        
    // 获取所有函数指针
    g_enumDevices = (SensorEnumDevices)dlsym(g_deviceSDK, "sensorEnumDevices");
    g_openDevice = (SensorOpen)dlsym(g_deviceSDK, "sensorOpen");
    g_closeDevice = (SensorClose)dlsym(g_deviceSDK, "sensorClose");
    g_capture = (SensorCapture)dlsym(g_deviceSDK, "sensorCapture");
    g_getParameter = (SensorGetParameter)dlsym(g_deviceSDK, "sensorGetParameter");
    g_status = (SensorStatus)dlsym(g_deviceSDK, "sensorStatus");
    
    // 检查所有函数是否加载成功
    if (!g_enumDevices || !g_openDevice || !g_closeDevice || 
        !g_capture || !g_getParameter || !g_status) {
        std::cout << "获取函数指针失败: " << dlerror() << std::endl;
        dlclose(g_deviceSDK);
        g_deviceSDK = nullptr;
        return false;
    }
    
    return true;
}

void FingerDevice::destroySDK() {
    if (g_deviceSDK) {
        dlclose(g_deviceSDK);
        g_deviceSDK = nullptr;
        g_enumDevices = nullptr;
        g_openDevice = nullptr;
        g_closeDevice = nullptr;
        g_capture = nullptr;
        g_getParameter = nullptr;
        g_status = nullptr;
    }
}

bool FingerDevice::isDeviceConnected() {
    deviceList_.resize(16);  // 文档建议的最大设备数
    int count = g_enumDevices(deviceList_.data(), deviceList_.size());
    std::cout << "检测到 " << count << " 个设备" << std::endl;
    return count > 0;
}

bool FingerDevice::openDevice() {
    if (isOpen_) {
        std::cout << "设备已经打开" << std::endl;
        return false;
    }
    
    if (deviceList_.empty()) {
        std::cout << "没有可用设备" << std::endl;
        return false;
    }
        
    deviceHandle_ = g_openDevice(&deviceList_[0]);
    if (!deviceHandle_) {
        std::cout << "打开设备失败" << std::endl;
        return false;
    }
    
    // 检查设备状态
    int status = g_status(deviceHandle_);
    if (status != 0) {
        std::cout << "设备状态异常: " << status << std::endl;
        g_closeDevice(deviceHandle_);
        deviceHandle_ = nullptr;
        return false;
    }
        
    isOpen_ = true;
    std::cout << "设备打开成功" << std::endl;
    return true;
}

bool FingerDevice::closeDevice() {
    if (!isOpen_) {
        std::cout << "设备未打开" << std::endl;
        return false;
    }
        
    int result = g_closeDevice(deviceHandle_);
    if (result == 0) {
        isOpen_ = false;
        deviceHandle_ = nullptr;
        std::cout << "设备关闭成功" << std::endl;
        return true;
    }
    
    std::cout << "设备关闭失败: " << result << std::endl;
    return false;
}

int FingerDevice::captureImage(unsigned char* buffer, int size) {
    if (!isOpen_) {
        std::cout << "设备未打开" << std::endl;
        return -1;
    }
    
    // 检查设备状态
    int status = g_status(deviceHandle_);
    if (status != 0) {
        std::cout << "设备状态异常: " << status << std::endl;
        return -1;
    }
    
    int result = g_capture(deviceHandle_, buffer, size);
    return result;
}

int FingerDevice::getParameter(int type) {
    if (!isOpen_) {
        std::cout << "设备未打开" << std::endl;
        return -1;
    }
    
    // 根据文档，type 1 表示宽度，type 2 表示高度
    if (type != 1 && type != 2) {
        std::cout << "无效的参数类型: " << type << std::endl;
        return -1;
    }
    
    int value = g_getParameter(deviceHandle_, type);
    return value;
}
