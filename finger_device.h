#pragma once
#include <string>
#include <vector>

struct DeviceInfo {
    unsigned short vid;
    unsigned short pid;
    unsigned char serialNumber[64];
    unsigned int busNumber;
    unsigned int deviceAddress;
    unsigned int extra;
};

class FingerDevice {
public:
    static bool initSDK();
    static void destroySDK();
    
    bool isDeviceConnected();
    bool openDevice();
    bool closeDevice();
    
    // 采集指纹图像
    int captureImage(unsigned char* buffer, int size);
    
    // 获取设备参数
    int getParameter(int type);
    
    // 获取设备列表
    const std::vector<DeviceInfo>& getDeviceList() const { return deviceList_; }

private:    
    std::vector<DeviceInfo> deviceList_;
    void* deviceHandle_ = nullptr;
    bool isOpen_ = false;
}; 
