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

    json::value processCommand(const json::value &body)
    {
        auto cmd = utility::conversions::to_utf8string(body.at(U("cmd")).as_string());
        json::value response;

        if (cmd == "isConnected")
        {
            bool isConnected = device_->isDeviceConnected();
            response[U("success")] = json::value::boolean(isConnected);
        }
        else if (cmd == "openDevice")
        {
            if (!device_)
            {
                throw std::runtime_error("Device not initialized");
            }

            bool success = device_->openDevice();
            if (success)
            {
                int width = device_->getParameter(1);
                int height = device_->getParameter(2);
                algorithmHandle_ = FingerAlgorithm::initAlgorithm(0, width, height, nullptr);
                success = algorithmHandle_ != nullptr;
            }

            response[U("success")] = json::value::boolean(success);
            if (!success)
            {
                response[U("error")] = json::value::string(U("Failed to initialize algorithm"));
                std::cout << "设备打开失败" << std::endl;
            }
            else
            {
                std::cout << "设备打开成功" << std::endl;
            }
        }
        else if (cmd == "closeDevice")
        {
            if (!device_)
            {
                throw std::runtime_error("Device not initialized");
            }

            bool success = device_->closeDevice();
            if (success && algorithmHandle_)
            {
                FingerAlgorithm::closeAlgorithm(algorithmHandle_);
                algorithmHandle_ = nullptr;
                std::cout << "设备关闭成功" << std::endl;
            }
            else
            {
                std::cout << "设备关闭失败" << std::endl;
            }

            response[U("success")] = json::value::boolean(success);
        }
        else if (cmd == "loadTemplates")
        {
            std::cout << "加载模板" << std::endl;
            try
            {
                auto templates = body.at(U("templates")).as_array();
                std::cout << "模板数据数组大小: " << templates.size() << std::endl;
                bool hasError = false;
                std::string errorMsg;

                for (const auto &templ : templates)
                {
                    int id = templ.at(U("id")).as_integer();
                    std::string templateData = utility::conversions::to_utf8string(templ.at(U("template")).as_string());
                    std::vector<unsigned char> templateBuffer = base64_decode(templateData);

                    if (templateBuffer.empty())
                    {
                        hasError = true;
                        errorMsg = "Invalid template data for id " + std::to_string(id);
                        break;
                    }

                    int result = FingerAlgorithm::addTemplateToDb(
                        algorithmHandle_,
                        id,
                        templateBuffer.size(),
                        templateBuffer.data());

                    std::cout << "加载模板结果: " << result << std::endl;
                    if (result <= 0)
                    {
                        hasError = true;
                        errorMsg = "Failed to load template for id " + std::to_string(id);
                        break;
                    }
                }

                response[U("success")] = json::value::boolean(!hasError);
                std::cout << "加载模板结果: " << (hasError ? "失败" : "成功") << std::endl;
                if (hasError)
                {
                    response[U("error")] = json::value::string(utility::conversions::to_string_t(errorMsg));
                }
            }
            catch (const json::json_exception &e)
            {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Invalid request parameters"));
            }
        }
        else if (cmd == "capture")
        {
            if (!device_ || !algorithmHandle_)
            {
                throw std::runtime_error("Device or algorithm not initialized");
            }

            int width = device_->getParameter(1);
            int height = device_->getParameter(2);
            if (width <= 0 || height <= 0)
            {
                throw std::runtime_error("Invalid device parameters");
            }

            std::vector<unsigned char> imageBuffer(width * height);
            int result = device_->captureImage(imageBuffer.data(), imageBuffer.size());

            if (result > 0)
            {
                std::vector<unsigned char> templateBuffer(2048);
                int templateLength = FingerAlgorithm::extractTemplate(algorithmHandle_, imageBuffer.data(), width, height, templateBuffer.data(), templateBuffer.size(), 0);
                if (templateLength > 0)
                {
                    response[U("success")] = json::value::boolean(true);
                    response[U("template")] = json::value::string(utility::conversions::to_string_t(base64_encode(templateBuffer)));
                }
                else
                {
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Feature extraction failed"));
                }
            }
            else
            {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Capture failed"));
            }
        }
        else if (cmd == "register")
        {
            std::cout << "注册指纹" << std::endl;
            if (!device_ || !algorithmHandle_)
            {
                throw std::runtime_error("Device or algorithm not initialized");
            }

            // 获取指纹模板数据数组
            auto fingerId = body.at(U("fingerId")).as_integer();
            std::cout << "指纹ID: " << fingerId << std::endl;
            auto templateDataArray = body.at(U("templateData")).as_array();
            std::cout << "模板数据数组大小: " << templateDataArray.size() << std::endl;

            if (templateDataArray.size() == 0)
            {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Empty template data array"));
                return response;
            }

            // 解码所有指纹模板
            std::vector<std::vector<unsigned char>> fingerTemplates;
            for (const auto &templateData : templateDataArray)
            {
                auto templateStr = templateData.as_string();
                fingerTemplates.push_back(base64_decode(utility::conversions::to_utf8string(templateStr)));
            }

            // 准备指纹模板指针数组
            std::vector<unsigned char *> templatePtrs;
            for (auto &fingerTemplate : fingerTemplates)
            {
                templatePtrs.push_back(fingerTemplate.data());
            }

            // 生成最终模板
            std::vector<unsigned char> finalTemplate(2048);
            int genResult = FingerAlgorithm::generateTemplate(
                algorithmHandle_,
                templatePtrs.data(),
                templatePtrs.size(),
                finalTemplate.data());

            if (genResult <= 0)
            {
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to generate final template"));
                return response;
            }

            // 添加到数据库
            int addResult = FingerAlgorithm::addTemplateToDb(
                algorithmHandle_,
                fingerId,
                genResult,
                finalTemplate.data());

            if (addResult != 1)
            {
                std::cout << "添加模板到数据库失败，错误码: " << addResult << std::endl;
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to add template to database"));
                return response;
            }

            std::cout << "成功添加模板到数据库" << std::endl;
            response[U("success")] = json::value::boolean(true);
            response[U("templateData")] = json::value::string(
                utility::conversions::to_string_t(base64_encode(finalTemplate)));
        }
        else if (cmd == "verify")
        {
            if (!algorithmHandle_)
            {
                throw std::runtime_error("Algorithm not initialized");
            }

            // 从请求中获取两个指纹模板数据
            auto templateData1 = body.at(U("templateData1")).as_string();
            auto templateData2 = body.at(U("templateData2")).as_string();

            std::vector<unsigned char> fingerTemplate1 = base64_decode(utility::conversions::to_utf8string(templateData1));
            std::vector<unsigned char> fingerTemplate2 = base64_decode(utility::conversions::to_utf8string(templateData2));

            // 比对两个指纹
            int score = FingerAlgorithm::verifyTemplate(
                algorithmHandle_,
                fingerTemplate1.data(),
                fingerTemplate2.data());

            // 分数超过50认为比对成功
            bool matched = score >= 50;
            response[U("success")] = json::value::boolean(matched);
            response[U("score")] = json::value::number(score);
        }
        else if (cmd == "identify")
        {
            std::cout << "识别指纹" << std::endl;
            if (!algorithmHandle_)
            {
                throw std::runtime_error("Algorithm not initialized");
            }

            // 从请求中获取指纹模板数据
            auto templateData = body.at(U("template")).as_string();
            std::vector<unsigned char> fingerTemplate = base64_decode(utility::conversions::to_utf8string(templateData));
            std::cout << "指纹模板数据大小: " << fingerTemplate.size() << std::endl;

            // 识别指纹
            int matchedId = 0;
            int score = 0;
            int identifyResult = FingerAlgorithm::identifyTemplate(
                algorithmHandle_,
                fingerTemplate.data(),
                &matchedId,
                &score);

            std::cout << "匹配的ID: " << matchedId << std::endl;
            std::cout << "匹配分数: " << score << std::endl;

            response[U("success")] = json::value::boolean(identifyResult == 1);
            std::cout << "识别结果: " << (identifyResult == 1 ? "成功" : "失败") << std::endl;
            response[U("matchedId")] = json::value::number(matchedId);
        }
        else
        {
            throw std::runtime_error("Unknown command");
        }

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
