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

private:    
    std::vector<DeviceInfo> deviceList_;
}; 
