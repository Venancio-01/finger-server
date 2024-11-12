#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "finger_device.h"
#include "finger_algorithm.h"
#include "base64.h"
#include <iostream>
#include <memory>
#include <iomanip>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class FingerServer
{
public:
    FingerServer() : device_(nullptr), algorithmHandle_(nullptr)
    {
        // 创建 HTTP 监听器
        listener_ = http_listener(U("http://0.0.0.0:22813"));

        // 注册请求处理函数
        listener_.support(methods::POST, std::bind(&FingerServer::handlePost, this, std::placeholders::_1));
    }

    bool start()
    {
        try
        {
            // 初始化 SDK
            bool deviceInit = FingerDevice::initSDK();
            bool algorithmInit = FingerAlgorithm::initSDK();

            if (deviceInit && algorithmInit)
            {
                device_ = std::make_unique<FingerDevice>();
                std::cout << "SDK 初始化成功" << std::endl;
            }
            else
            {
                std::cout << "SDK 初始化失败" << std::endl;
                return false;
            }

            // 启动 HTTP 服务
            listener_.open().wait();
            std::cout << "指纹服务已启动，监听端口: 22813" << std::endl;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cout << "服务启动失败: " << e.what() << std::endl;
            return false;
        }
    }

    void stop()
    {
        try
        {
            // 关闭 HTTP 服务
            listener_.close().wait();

            // 清理资源
            if (algorithmHandle_)
            {
                FingerAlgorithm::closeAlgorithm(algorithmHandle_);
                algorithmHandle_ = nullptr;
            }
            if (device_)
            {
                device_->closeDevice();
                device_.reset();
            }

            // 销毁 SDK
            FingerDevice::destroySDK();
            FingerAlgorithm::destroySDK();

            std::cout << "服务已停止，资源已清理" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cout << "服务停止时发生错误: " << e.what() << std::endl;
        }
    }

private:
    void handlePost(http_request request)
    {
        try
        {
            // 获取请求体
            request.extract_json().then([this, request](json::value body)
                                        {
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
                } })
                .wait();
        }
        catch (const std::exception &e)
        {
            json::value error;
            error[U("success")] = json::value::boolean(false);
            error[U("error")] = json::value::string(utility::conversions::to_string_t(e.what()));
            request.reply(status_codes::BadRequest, error);
        }
    }

    json::value processCommand(const json::value& body) {
        auto cmd = utility::conversions::to_utf8string(body.at(U("cmd")).as_string());
        std::cout << "\n收到命令: " << cmd << std::endl;
        json::value response;

        if (cmd == "isConnected") {
            bool isConnected = device_->isDeviceConnected();
            response[U("success")] = json::value::boolean(isConnected);
        }
        else if (cmd == "openDevice") {
            if (!device_) {
                std::cout << "错误: 设备未初始化" << std::endl;
                throw std::runtime_error("Device not initialized");
            }

            bool success = device_->openDevice();
            if (success) {
                int width = device_->getParameter(1);  // 1=宽度
                int height = device_->getParameter(2); // 2=高度
                std::cout << "设备参数 - 宽度: " << width << ", 高度: " << height << std::endl;

                algorithmHandle_ = FingerAlgorithm::initAlgorithm(0, width, height, nullptr);
                success = algorithmHandle_ != nullptr;
            }

            response[U("success")] = json::value::boolean(success);
            if (!success) {
                response[U("error")] = json::value::string(U("Failed to initialize device or algorithm"));
            }
        }
        else if (cmd == "capture") {
            if (!device_ || !algorithmHandle_) {
                std::cout << "错误: 设备或算法未初始化" << std::endl;
                throw std::runtime_error("Device or algorithm not initialized");
            }

            int width = device_->getParameter(1);
            int height = device_->getParameter(2);
            std::cout << "开始采集指纹图像 (分辨率: " << width << "x" << height << ")" << std::endl;

            std::vector<unsigned char> imageBuffer(width * height);
            int captureResult = device_->captureImage(imageBuffer.data(), imageBuffer.size());

            if (captureResult > 0) {
                std::vector<unsigned char> templateBuffer(2048);  // 按文档建议分配 2048 字节
                int templateLength = FingerAlgorithm::extractTemplate(
                    algorithmHandle_,
                    imageBuffer.data(),
                    width,
                    height,
                    templateBuffer.data(),
                    templateBuffer.size(),
                    0  // flag 固定为 0
                );

                if (templateLength > 0) {
                    response[U("success")] = json::value::boolean(true);
                    response[U("template")] = json::value::string(
                        utility::conversions::to_string_t(base64_encode(templateBuffer)));
                    response[U("length")] = json::value::number(templateLength);
                } else {
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Feature extraction failed"));
                }
            } else {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Capture failed"));
            }
        }
        else if (cmd == "verify") {
            if (!algorithmHandle_) {
                std::cout << "错误: 算法未初始化" << std::endl;
                throw std::runtime_error("Algorithm not initialized");
            }

            auto templateData1 = body.at(U("templateData1")).as_string();
            auto templateData2 = body.at(U("templateData2")).as_string();

            std::vector<unsigned char> fingerTemplate1 = base64_decode(
                utility::conversions::to_utf8string(templateData1));
            std::vector<unsigned char> fingerTemplate2 = base64_decode(
                utility::conversions::to_utf8string(templateData2));

            int score = FingerAlgorithm::verifyTemplate(
                algorithmHandle_,
                fingerTemplate1.data(),
                fingerTemplate2.data());

            bool matched = score >= 50;  // 按文档推荐阈值
            response[U("success")] = json::value::boolean(matched);
            response[U("score")] = json::value::number(score);
        }
        else if (cmd == "identify") {
            if (!algorithmHandle_) {
                std::cout << "错误: 算法未初始化" << std::endl;
                throw std::runtime_error("Algorithm not initialized");
            }

            auto templateData = body.at(U("template")).as_string();
            std::vector<unsigned char> fingerTemplate = base64_decode(
                utility::conversions::to_utf8string(templateData));

            int matchedId = 0;
            int score = 0;
            int result = FingerAlgorithm::identifyTemplate(
                algorithmHandle_,
                fingerTemplate.data(),
                &matchedId,
                &score);

            response[U("success")] = json::value::boolean(result == 1);
            response[U("matchedId")] = json::value::number(matchedId);
            response[U("score")] = json::value::number(score);
            // 添加阈值提示
            response[U("threshold")] = json::value::number(70);  // 文档推荐阈值
        }
        else if (cmd == "register") {
            if (!algorithmHandle_) {
                std::cout << "错误: 算法未初始化" << std::endl;
                throw std::runtime_error("Algorithm not initialized");
            }

            auto fingerId = body.at(U("fingerId")).as_integer();
            auto templateDataArray = body.at(U("templateData")).as_array();

            std::vector<std::vector<unsigned char>> fingerTemplates;
            std::vector<unsigned char*> templatePtrs;

            for (const auto& templateData : templateDataArray) {
                auto templateStr = templateData.as_string();
                fingerTemplates.push_back(base64_decode(
                    utility::conversions::to_utf8string(templateStr)));
                templatePtrs.push_back(fingerTemplates.back().data());
            }

            std::vector<unsigned char> finalTemplate(2048);  // 按文档建议分配 2048 字节
            int genResult = FingerAlgorithm::generateTemplate(
                algorithmHandle_,
                templatePtrs.data(),
                templatePtrs.size(),
                finalTemplate.data());

            if (genResult > 0) {
                int addResult = FingerAlgorithm::addTemplateToDb(
                    algorithmHandle_,
                    fingerId,
                    genResult,
                    finalTemplate.data());

                if (addResult > 0) {
                    response[U("success")] = json::value::boolean(true);
                    response[U("templateData")] = json::value::string(
                        utility::conversions::to_string_t(base64_encode(finalTemplate)));
                } else {
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Failed to add template to database"));
                }
            } else {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to generate template"));
            }
        }
        else if (cmd == "identify")
        {
            if (!algorithmHandle_) {
                std::cout << "错误: 算法未初始化" << std::endl;
                throw std::runtime_error("Algorithm not initialized");
            }

            auto templateData = body.at(U("template")).as_string();
            std::vector<unsigned char> fingerTemplate = base64_decode(
                utility::conversions::to_utf8string(templateData));

            int matchedId = 0;
            int score = 0;
            int result = FingerAlgorithm::identifyTemplate(
                algorithmHandle_,
                fingerTemplate.data(),
                &matchedId,
                &score);

            response[U("success")] = json::value::boolean(result == 1);
            response[U("matchedId")] = json::value::number(matchedId);
            response[U("score")] = json::value::number(score);
        }
        else {
            std::cout << "错误: 未知命令 " << cmd << std::endl;
            throw std::runtime_error("Unknown command");
        }

        std::cout << "命令执行完成，返回结果: " << response.serialize() << std::endl;
        return response;
    }

    http_listener listener_;
    std::unique_ptr<FingerDevice> device_;
    void *algorithmHandle_;
};

int main()
{
    FingerServer server;

    // 启动服务
    if (!server.start())
    {
        return 1;
    }

    std::cout << "按 Enter 键退出..." << std::endl;
    std::string line;
    std::getline(std::cin, line);

    // 停止服务
    server.stop();
    return 0;
}
