#include "finger_device.h"
#include "finger_algorithm.h"
#include "base64.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <dlfcn.h>

using json = nlohmann::json;

class CommandProcessor {
public:
    CommandProcessor() : device_(nullptr), algorithmHandle_(nullptr) {}
    
    void processCommand(const std::string& input) {
        try {
            auto command = json::parse(input);
            std::string cmd = command["cmd"];
            
            json response;
            // 初始化 SDK
            if (cmd == "initialize") {
                bool deviceInit = FingerDevice::initSDK();
                bool algorithmInit = FingerAlgorithm::initSDK();
                
                bool success = deviceInit && algorithmInit;
                if (success) {
                    device_ = std::make_unique<FingerDevice>();
                }
                
                response = {
                    {"success", success},
                    {"deviceInit", deviceInit},
                    {"algorithmInit", algorithmInit}
                };
            }
            // 打开设备并初始化算法
            else if (cmd == "openDevice") {
                if (!device_) {
                    response = {
                        {"success", false},
                        {"error", "Device not initialized"}
                    };
                } else {
                    bool success = device_->openDevice();
                    
                    if (success) {
                        // 获取设备参数
                        int width = device_->getParameter(1);
                        int height = device_->getParameter(2);
                        
                        // 检查 SDK 是否正确加载
                        void* sdkHandle = dlopen("libzkfp.so", RTLD_LAZY);
                        if (!sdkHandle) {
                            success = false;
                        } else {
                            dlclose(sdkHandle);
                        }
                        
                        algorithmHandle_ = FingerAlgorithm::initAlgorithm(0, width, height, nullptr);
                        success = algorithmHandle_ != nullptr;
                    }
                    
                    response = {
                        {"success", success},
                        {"error", success ? "" : "Failed to initialize algorithm"},
                        {"debug_info", {
                            {"device_connected", device_->isDeviceConnected()},
                            {"width", device_->getParameter(1)},
                            {"height", device_->getParameter(2)},
                            {"algorithm_handle", algorithmHandle_ != nullptr}
                        }}
                    };
                }
            }
            // 检查设备是否连接
            else if (cmd == "isConnected") {
                bool connected = device_ ? device_->isDeviceConnected() : false;
                response = {
                    {"success", true},
                    {"connected", connected}
                };
            }
            // 关闭设备
            else if (cmd == "closeDevice") {
                if (!device_) {
                    response = {
                        {"success", false},
                        {"error", "Device not initialized"}
                    };
                } else {
                    bool success = device_->closeDevice();
                    if (success && algorithmHandle_) {
                        // 关闭算法
                        FingerAlgorithm::closeAlgorithm(algorithmHandle_);
                        algorithmHandle_ = nullptr;
                    }
                    response = {
                        {"success", success}
                    };
                }
            }
            // 添加指纹模板
            else if (cmd == "addTemplate") {
                if (!algorithmHandle_) {
                    response = {
                        {"success", false},
                        {"error", "Algorithm not initialized"}
                    };
                } else {
                    int id = command["id"];
                    int length = command["length"];
                    std::string dataBase64 = command["data"];
                    std::vector<unsigned char> data = base64_decode(dataBase64);
                    
                    int result = FingerAlgorithm::addTemplateToDb(algorithmHandle_, id, length, data.data());
                    response = {
                        {"success", result == 1},
                        {"result", result}
                    };
                }
            }
            // 识别指纹
            else if (cmd == "identify") {
                if (!algorithmHandle_) {
                    response = {
                        {"success", false},
                        {"error", "Algorithm not initialized"}
                    };
                } else {
                    std::string dataBase64 = command["data"];
                    std::vector<unsigned char> data = base64_decode(dataBase64);
                    int matchedId = 0;
                    int score = 0;
                    
                    int result = FingerAlgorithm::identifyTemplate(algorithmHandle_, data.data(), &matchedId, &score);
                    response = {
                        {"success", result == 1},
                        {"matchedId", matchedId},
                        {"score", score}
                    };
                }
            }
            // 采集指纹图像
            else if (cmd == "capture") {
                if (!device_) {
                    response = {
                        {"success", false},
                        {"error", "Device not initialized"}
                    };
                } else {
                    // 获取设备参数
                    int width = device_->getParameter(1);  // 1 表示宽度
                    int height = device_->getParameter(2); // 2 表示高度
                    if (width <= 0 || height <= 0) {
                        response = {
                            {"success", false},
                            {"error", "Invalid device parameters"}
                        };
                    } else {
                        std::vector<unsigned char> buffer(width * height);
                        int result = device_->captureImage(buffer.data(), buffer.size());
                        if (result > 0) {
                            response = {
                                {"success", true},
                                {"width", width},
                                {"height", height},
                                {"data", base64_encode(buffer)}
                            };
                        } else {
                            response = {
                                {"success", false},
                                {"error", "Capture failed"}
                            };
                        }
                    }
                }
            }
            // 销毁 SDK
            else if (cmd == "uninitialize") {
                if (algorithmHandle_) {
                    FingerAlgorithm::closeAlgorithm(algorithmHandle_);
                    algorithmHandle_ = nullptr;
                }
                if (device_) {
                    device_->closeDevice();
                    device_.reset();
                }
                FingerDevice::destroySDK();
                FingerAlgorithm::destroySDK();
                
                response = {
                    {"success", true}
                };
            }
            
            std::cout << response.dump() << std::endl;
            
        } catch (const std::exception& e) {
            json error = {
                {"success", false},
                {"error", e.what()}
            };
            std::cout << error.dump() << std::endl;
        }
    }
    
    ~CommandProcessor() {
        if (algorithmHandle_) {
            FingerAlgorithm::closeAlgorithm(algorithmHandle_);
            algorithmHandle_ = nullptr;
        }
        FingerDevice::destroySDK();
        FingerAlgorithm::destroySDK();
    }
    
private:
    std::unique_ptr<FingerDevice> device_;
    void* algorithmHandle_;
};

int main() {
    CommandProcessor processor;
    std::string line;
    
    while (std::getline(std::cin, line)) {
        processor.processCommand(line);
    }
    
    return 0;
} 
