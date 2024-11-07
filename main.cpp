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
        else if (cmd == "closeDevice") {
            if (!device_) {
                throw std::runtime_error("Device not initialized");
            }
            
            bool success = device_->closeDevice();
            if (success && algorithmHandle_) {
                FingerAlgorithm::closeAlgorithm(algorithmHandle_);
                algorithmHandle_ = nullptr;
            }
            
            response[U("success")] = json::value::boolean(success);
        }
        else if (cmd == "capture") {
            if (!device_ || !algorithmHandle_) {
                throw std::runtime_error("Device or algorithm not initialized");
            }
            
            int width = device_->getParameter(1);
            int height = device_->getParameter(2);
            if (width <= 0 || height <= 0) {
                throw std::runtime_error("Invalid device parameters");
            }
            
            std::vector<unsigned char> buffer(width * height);
            int result = device_->captureImage(buffer.data(), buffer.size());
            
            if (result > 0) {
                response[U("success")] = json::value::boolean(true);
                response[U("width")] = json::value::number(width);
                response[U("height")] = json::value::number(height);
                response[U("data")] = json::value::string(utility::conversions::to_string_t(base64_encode(buffer)));
            } else {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Capture failed"));
            }
        }
        else if (cmd == "register") {
            if (!device_ || !algorithmHandle_) {
                throw std::runtime_error("Device or algorithm not initialized");
            }
            
            int index = body.at(U("index")).as_integer();
            static std::vector<std::vector<unsigned char>> registerTemplates;
            
            // 采集指纹图像
            int width = device_->getParameter(1);
            int height = device_->getParameter(2);
            std::vector<unsigned char> imageBuffer(width * height);
            int captureResult = device_->captureImage(imageBuffer.data(), imageBuffer.size());
            
            if (captureResult <= 0) {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to capture fingerprint"));
                return response;
            }
            
            // 提取特征
            std::vector<unsigned char> fingerTemplate(2048);
            int templateLength = FingerAlgorithm::extractTemplate(
                algorithmHandle_, 
                imageBuffer.data(), 
                width, 
                height, 
                fingerTemplate.data(), 
                2048, 
                0
            );
            
            if (templateLength <= 0) {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to extract template"));
                return response;
            }
            
            // 如果不是第一次采集，需要验证指纹
            if (index > 0) {
                bool isMatched = FingerAlgorithm::verifyTemplate(
                    algorithmHandle_,
                    registerTemplates[index - 1].data(),
                    fingerTemplate.data()
                );
                
                if (!isMatched) {
                    registerTemplates.clear();
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Please press the same finger"));
                    return response;
                }
            }
            
            // 保存模板
            registerTemplates.push_back(fingerTemplate);
            
            // 如果是最后一次采集，生成最终模板
            if (index == 2) {
                std::vector<unsigned char*> templatePtrs;
                for (auto& templ : registerTemplates) {
                    templatePtrs.push_back(templ.data());
                }
                
                std::vector<unsigned char> finalTemplate(2048);
                int genResult = FingerAlgorithm::generateTemplate(
                    algorithmHandle_,
                    templatePtrs.data(),
                    registerTemplates.size(),
                    finalTemplate.data()
                );
                
                if (genResult <= 0) {
                    registerTemplates.clear();
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Failed to generate final template"));
                    return response;
                }
                
                // 添加到数据库
                int addResult = FingerAlgorithm::addTemplateToDb(
                    algorithmHandle_,
                    9999,
                    genResult,
                    finalTemplate.data()
                );
                
                registerTemplates.clear();
                
                if (addResult != 1) {
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Failed to add template to database"));
                    return response;
                }
                
                response[U("success")] = json::value::boolean(true);
                response[U("templateData")] = json::value::string(
                    utility::conversions::to_string_t(base64_encode(finalTemplate))
                );
            } else {
                response[U("success")] = json::value::boolean(true);
                response[U("remainingCount")] = json::value::number(2 - index);
            }
        }
        else if (cmd == "identify") {
            if (!device_ || !algorithmHandle_) {
                throw std::runtime_error("Device or algorithm not initialized");
            }
            
            // 采集指纹图像
            int width = device_->getParameter(1);
            int height = device_->getParameter(2);
            std::vector<unsigned char> imageBuffer(width * height);
            int captureResult = device_->captureImage(imageBuffer.data(), imageBuffer.size());
            
            if (captureResult <= 0) {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to capture fingerprint"));
                return response;
            }
            
            // 提取特征
            std::vector<unsigned char> fingerTemplate(2048);
            int templateLength = FingerAlgorithm::extractTemplate(
                algorithmHandle_,
                imageBuffer.data(),
                width,
                height,
                fingerTemplate.data(),
                2048,
                0
            );
            
            if (templateLength <= 0) {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to extract template"));
                return response;
            }
            
            // 识别指纹
            int matchedId = 0;
            int score = 0;
            int identifyResult = FingerAlgorithm::identifyTemplate(
                algorithmHandle_,
                fingerTemplate.data(),
                &matchedId,
                &score
            );
            
            response[U("success")] = json::value::boolean(identifyResult == 1);
            response[U("matchedId")] = json::value::number(matchedId);
            response[U("score")] = json::value::number(score);
        }
        else if (cmd == "destroy") {
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
            
            response[U("success")] = json::value::boolean(true);
        }
        else {
            throw std::runtime_error("Unknown command");
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

