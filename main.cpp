#include "finger_device.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class CommandProcessor {
public:
    void processCommand(const std::string& input) {
        try {
            auto command = json::parse(input);
            std::string cmd = command["cmd"];
            
            json response;
            if (cmd == "init") {
                bool success = FingerDevice::initSDK();
                if (success) {
                    device_ = std::make_unique<FingerDevice>();
                }
                response = {
                    {"success", success}
                };
            }
            else if (cmd == "isConnected") {
                bool connected = device_ ? device_->isDeviceConnected() : false;
                response = {
                    {"success", true},
                    {"connected", connected}
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
        FingerDevice::destroySDK();
    }
    
private:
    std::unique_ptr<FingerDevice> device_;
};

int main() {
    CommandProcessor processor;
    std::string line;
    
    while (std::getline(std::cin, line)) {
        processor.processCommand(line);
    }
    
    return 0;
} 
