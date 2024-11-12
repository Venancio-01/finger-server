#pragma once
#include <string>
#include <vector>

// 设备信息结构体，与 SDK 文档一致
struct DeviceInfo {
    unsigned short vid;        // 设备厂商ID
    unsigned short pid;        // 设备产品ID
    unsigned char serialNumber[64];  // 设备序列号
    unsigned int busNumber;    // USB总线号
    unsigned int deviceAddress;  // 设备地址
    unsigned int extra;        // 扩展字段
};

class FingerDevice {
public:
    static bool initSDK();    // 初始化设备 SDK
    static void destroySDK(); // 销毁设备 SDK
    
    // 设备操作
    bool isDeviceConnected(); // 检查设备连接状态
    bool openDevice();        // 打开设备
    bool closeDevice();       // 关闭设备
    
    // 采集指纹图像，返回值 >0 表示成功并返回数据长度
    int captureImage(unsigned char* buffer, int size);
    
    // 获取设备参数
    // type: 1=宽度, 2=高度
    int getParameter(int type);
    
    // 获取设备列表
    const std::vector<DeviceInfo>& getDeviceList() const { return deviceList_; }
    
    // 检查设备是否已打开
    bool isOpen() const { return isOpen_; }

private:    
    std::vector<DeviceInfo> deviceList_;  // 设备列表
    void* deviceHandle_ = nullptr;        // 设备句柄
    bool isOpen_ = false;                 // 设备打开状态
}; 
