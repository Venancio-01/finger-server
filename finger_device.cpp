#include "finger_device.h"
#include "logger.h"
#include <dlfcn.h>
#include <iostream>
#include <sstream>

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
        LOG_INFO("加载设备 SDK 失败: " + std::string(dlerror()));
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
        LOG_INFO("获取函数指针失败: " + std::string(dlerror()));
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
        LOG_INFO("设备 SDK 已销毁");
    }
}

bool FingerDevice::isDeviceConnected() {
    deviceList_.resize(16);  // 文档建议的最大设备数
    int count = g_enumDevices(deviceList_.data(), deviceList_.size());
    LOG_INFO("检测到 " + std::to_string(count) + " 个设备");
    return count > 0;
}

bool FingerDevice::openDevice() {
    if (isOpen_) {
        LOG_INFO("设备已经打开");
        return false;
    }
    
    if (deviceList_.empty()) {
        LOG_INFO("没有可用设备");
        return false;
    }
        
    deviceHandle_ = g_openDevice(&deviceList_[0]);
    if (!deviceHandle_) {
        LOG_INFO("打开设备失败");
        return false;
    }
    
    // 检查设备状态
    int status = g_status(deviceHandle_);
    if (status != 0) {
        LOG_INFO("设备状态异常: " + std::to_string(status));
        g_closeDevice(deviceHandle_);
        deviceHandle_ = nullptr;
        return false;
    }
        
    isOpen_ = true;
    LOG_INFO("设备打开成功");
    return true;
}

bool FingerDevice::closeDevice() {
    if (!isOpen_) {
        LOG_INFO("设备未打开");
        return false;
    }
        
    int result = g_closeDevice(deviceHandle_);
    if (result == 0) {
        isOpen_ = false;
        deviceHandle_ = nullptr;
        LOG_INFO("设备关闭成功");
        return true;
    }
    
    LOG_INFO("设备关闭失败: " + std::to_string(result));
    return false;
}

int FingerDevice::captureImage(unsigned char* buffer, int size) {
    if (!isOpen_) {
        LOG_INFO("设备未打开");
        return -1;
    }
    
    // 检查设备状态
    int status = g_status(deviceHandle_);
    if (status != 0) {
        LOG_INFO("设备状态异常: " + std::to_string(status));
        return -1;
    }
    
    int result = g_capture(deviceHandle_, buffer, size);
    if (result <= 0) {
        LOG_INFO("采集图像失败: " + std::to_string(result));
    }
    return result;
}

int FingerDevice::getParameter(int type) {
    if (!isOpen_) {
        LOG_INFO("设备未打开");
        return -1;
    }
    
    // 根据文档，type 1 表示宽度，type 2 表示高度
    if (type != 1 && type != 2) {
        LOG_INFO("无效的参数类型: " + std::to_string(type));
        return -1;
    }
    
    int value = g_getParameter(deviceHandle_, type);
    if (value <= 0) {
        LOG_INFO("获取参数失败: type=" + std::to_string(type) + ", result=" + std::to_string(value));
    }
    return value;
}
