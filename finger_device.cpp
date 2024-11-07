#include "finger_device.h"
#include <dlfcn.h>
#include <iostream>
// SDK 函数指针定义
typedef int (*SensorEnumDevices)(DeviceInfo*, int);
typedef void* (*SensorOpen)(DeviceInfo*);
typedef int (*SensorClose)(void*);
typedef int (*SensorCapture)(void*, unsigned char*, int);
typedef int (*SensorGetParameter)(void*, int);

static void* g_deviceSDK = nullptr;
static SensorEnumDevices g_enumDevices = nullptr;
static SensorOpen g_openDevice = nullptr;
static SensorClose g_closeDevice = nullptr;
static SensorCapture g_capture = nullptr;
static SensorGetParameter g_getParameter = nullptr;

bool FingerDevice::initSDK() {
    g_deviceSDK = dlopen("libzkfpcap.so", RTLD_LAZY);
    if (!g_deviceSDK)
        return false;
        
    g_enumDevices = (SensorEnumDevices)dlsym(g_deviceSDK, "sensorEnumDevices");
    g_openDevice = (SensorOpen)dlsym(g_deviceSDK, "sensorOpen");
    g_closeDevice = (SensorClose)dlsym(g_deviceSDK, "sensorClose");
    g_capture = (SensorCapture)dlsym(g_deviceSDK, "sensorCapture");
    g_getParameter = (SensorGetParameter)dlsym(g_deviceSDK, "sensorGetParameter");
    
    return g_enumDevices != nullptr && g_openDevice != nullptr && 
           g_closeDevice != nullptr && g_capture != nullptr && 
           g_getParameter != nullptr;
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
    }
}

bool FingerDevice::isDeviceConnected() {
    deviceList_.resize(16);
    int count = g_enumDevices(deviceList_.data(), deviceList_.size());
    return count > 0;
}

bool FingerDevice::openDevice() {
    if (isOpen_ || deviceList_.empty())
        return false;
        
    deviceHandle_ = g_openDevice(&deviceList_[0]);
    std::cout << "设备句柄: " << deviceHandle_ << std::endl;
    if (!deviceHandle_)
        return false;
        
    isOpen_ = true;
    return true;
}

bool FingerDevice::closeDevice() {
    if (!isOpen_)
        return false;
        
    int result = g_closeDevice(deviceHandle_);
    if (result == 0) {
        isOpen_ = false;
        deviceHandle_ = nullptr;
        return true;
    }
    return false;
}

int FingerDevice::captureImage(unsigned char* buffer, int size) {
    if (!isOpen_)
        return -1;
    return g_capture(deviceHandle_, buffer, size);
}

int FingerDevice::getParameter(int type) {
    if (!isOpen_)
        return -1;
    return g_getParameter(deviceHandle_, type);
}
