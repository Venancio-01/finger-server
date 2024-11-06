#include "finger_device.h"
#include <dlfcn.h>

// 只保留枚举设备的函数指针定义
typedef int (*SensorEnumDevices)(DeviceInfo*, int);

static void* g_deviceSDK = nullptr;
static SensorEnumDevices g_enumDevices = nullptr;

bool FingerDevice::initSDK() {
    g_deviceSDK = dlopen("libzkfpcap.so", RTLD_LAZY);
    if (!g_deviceSDK)
        return false;
        
    g_enumDevices = (SensorEnumDevices)dlsym(g_deviceSDK, "sensorEnumDevices");
    return g_enumDevices != nullptr;
}

void FingerDevice::destroySDK() {
    if (g_deviceSDK) {
        dlclose(g_deviceSDK);
        g_deviceSDK = nullptr;
    }
}

bool FingerDevice::isDeviceConnected() {
    deviceList_.resize(16);
    int count = g_enumDevices(deviceList_.data(), deviceList_.size());
    return count > 0;
}
