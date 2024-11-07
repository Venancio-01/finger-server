#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "finger_device.h"
#include "finger_algorithm.h"
#include "base64.h"
#include <iostream>
#include <memory>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class FingerServer {
public:
    FingerServer() : device_(nullptr), algorithmHandle_(nullptr) {
        // 创建 HTTP 监听器
        listener_ = http_listener(U("http://0.0.0.0:22813"));
        
        // 注册请求处理函数
        listener_.support(methods::POST, std::bind(&FingerServer::handlePost, this, std::placeholders::_1));
    }
    
    void start() {
        try {
            listener_.open().wait();
            std::cout << "指纹服务已启动，监听端口: 22813" << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "服务启动失败: " << e.what() << std::endl;
        }
    }
    
    void stop() {
        listener_.close().wait();
    }
    
private:
    void handlePost(http_request request) {
        try {
            // 获取请求体
            request.extract_json().then([this, request](json::value body) {
                try {
                    auto response = processCommand(body);
                    http_response resp(status_codes::OK);
                    resp.headers().add(U("Content-Type"), U("application/json"));
                    resp.set_body(response);
                    request.reply(resp);
                }
                catch (const std::exception& e) {
                    json::value error;
                    error[U("success")] = json::value::boolean(false);
                    error[U("error")] = json::value::string(utility::conversions::to_string_t(e.what()));
                    request.reply(status_codes::BadRequest, error);
                }
            }).wait();
        }
        catch (const std::exception& e) {
            json::value error;
            error[U("success")] = json::value::boolean(false);
            error[U("error")] = json::value::string(utility::conversions::to_string_t(e.what()));
            request.reply(status_codes::BadRequest, error);
        }
    }
    
    json::value processCommand(const json::value& body) {
        auto cmd = utility::conversions::to_utf8string(body.at(U("cmd")).as_string());
        json::value response;
        
        if (cmd == "init") {
            bool deviceInit = FingerDevice::initSDK();
            bool algorithmInit = FingerAlgorithm::initSDK();
            
            bool success = deviceInit && algorithmInit;
            if (success) {
                device_ = std::make_unique<FingerDevice>();
            }
            
            response[U("success")] = json::value::boolean(success);
            response[U("deviceInit")] = json::value::boolean(deviceInit);
            response[U("algorithmInit")] = json::value::boolean(algorithmInit);
        }
        else if (cmd == "isConnected") {
            bool isConnected = device_->isDeviceConnected();
            response[U("success")] = json::value::boolean(isConnected);
        }
        else if (cmd == "openDevice") {
            if (!device_) {
                throw std::runtime_error("Device not initialized");
            }
            
            bool success = device_->openDevice();
            if (success) {
                int width = device_->getParameter(1);
                int height = device_->getParameter(2);
                algorithmHandle_ = FingerAlgorithm::initAlgorithm(0, width, height, nullptr);
                success = algorithmHandle_ != nullptr;
            }
            
            response[U("success")] = json::value::boolean(success);
            if (!success) {
                response[U("error")] = json::value::string(U("Failed to initialize algorithm"));
            }
        }
        
        return response;
    }
    
    http_listener listener_;
    std::unique_ptr<FingerDevice> device_;
    void* algorithmHandle_;
};

int main() {
    FingerServer server;
    server.start();
    
    std::cout << "按 Enter 键退出..." << std::endl;
    std::string line;
    std::getline(std::cin, line);
    
    server.stop();
    return 0;
} 

