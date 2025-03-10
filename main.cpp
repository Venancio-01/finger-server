#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "finger_device.h"
#include "finger_algorithm.h"
#include "base64.h"
#include "logger.h"
#include <iostream>
#include <memory>
#include <iomanip>
#include <condition_variable>
#include <mutex>
#include <signal.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <chrono>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class FingerServer
{
public:
    FingerServer() : device_(nullptr), algorithmHandle_(nullptr), logger_("/var/log/finger_server/")
    {
        // 设置全局日志实例
        ILogger::setInstance(&logger_);
        LOG_INFO("指纹服务初始化开始");
        
        // 创建 HTTP 监听器
        listener_ = http_listener(U("http://0.0.0.0:22813"));

        // 注册请求处理函数
        listener_.support(methods::POST, std::bind(&FingerServer::handlePost, this, std::placeholders::_1));
        
        LOG_INFO("HTTP 监听器已创建");
    }

    ~FingerServer() {
        // 清除全局日志实例
        if (ILogger::getInstance() == &logger_) {
            ILogger::setInstance(nullptr);
        }
    }

    bool start()
    {
        try
        {
            LOG_INFO("开始初始化 SDK...");
            // 初始化 SDK
            bool deviceInit = FingerDevice::initSDK();
            bool algorithmInit = FingerAlgorithm::initSDK();

            if (deviceInit && algorithmInit)
            {
                device_ = std::make_unique<FingerDevice>();
                LOG_INFO("SDK 初始化成功");
            }
            else
            {
                LOG_ERROR("SDK 初始化失败");
                return false;
            }

            // 启动 HTTP 服务
            listener_.open().wait();
            LOG_INFO("指纹服务已启动，监听端口: 22813");
            handleTest();
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("服务启动失败: " + std::string(e.what()));
            return false;
        }
    }

    void stop()
    {
        try
        {
            LOG_INFO("开始停止服务...");
            // 关闭 HTTP 服务
            listener_.close().wait();

            // 清理资源
            if (algorithmHandle_)
            {
                LOG_INFO("正在关闭算法模块...");
                FingerAlgorithm::closeAlgorithm(algorithmHandle_);
                algorithmHandle_ = nullptr;
            }
            if (device_)
            {
                LOG_INFO("正在关闭设备...");
                device_->closeDevice();
                device_.reset();
            }

            // 销毁 SDK
            LOG_INFO("正在销毁 SDK...");
            FingerDevice::destroySDK();
            FingerAlgorithm::destroySDK();

            LOG_INFO("服务已停止，资源已清理");
            
            // 关闭日志文件
            logger_.close();
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("服务停止时发生错误: " + std::string(e.what()));
        }
    }

private:
    FileLogger logger_;
    http_listener listener_;
    std::unique_ptr<FingerDevice> device_;
    void *algorithmHandle_;


    void handleTest()
    {
            bool isConnected = device_->isDeviceConnected();
            LOG_INFO("设备连接状态: " + std::string(isConnected ? "已连接" : "未连接"));

            bool success = device_->openDevice();
            LOG_INFO("设备打开状态: " + std::string(success ? "成功" : "失败"));
    }

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
        if (cmd != "capture")
        {
            LOG_INFO("收到命令: " + cmd);
        }
        json::value response;

        if (cmd == "isConnected")
        {
            bool isConnected = device_->isDeviceConnected();
            response[U("success")] = json::value::boolean(isConnected);
            LOG_INFO("设备连接状态: " + std::string(isConnected ? "已连接" : "未连接"));
        }
        else if (cmd == "openDevice")
        {
            if (!device_)
            {
                LOG_ERROR("错误: 设备未初始化");
                throw std::runtime_error("Device not initialized");
            }

            bool success = device_->openDevice();
            if (success)
            {
                int width = device_->getParameter(1);  // 1=宽度
                int height = device_->getParameter(2); // 2=高度
                LOG_INFO("设备参数 - 宽度: " + std::to_string(width) + ", 高度: " + std::to_string(height));

                algorithmHandle_ = FingerAlgorithm::initAlgorithm(0, width, height, nullptr);
                success = algorithmHandle_ != nullptr;
                
                if (success) {
                    LOG_INFO("设备打开成功，算法初始化成功");
                } else {
                    LOG_INFO("设备打开成功，但算法初始化失败");
                }
            }
            else {
                LOG_ERROR("设备打开失败");
            }

            response[U("success")] = json::value::boolean(success);
            if (!success)
            {
                response[U("error")] = json::value::string(U("Failed to initialize device or algorithm"));
            }
        }
        else if (cmd == "closeDevice")
        {
            if (!device_)
            {
                LOG_ERROR("错误: 设备未初始化");
                throw std::runtime_error("Device not initialized");
            }

            bool success = device_->closeDevice();
            if (success)
            {
                LOG_INFO("设备已成功关闭");
            }
            else {
                LOG_ERROR("设备关闭失败");
            }

            response[U("success")] = json::value::boolean(success);
            if (!success)
            {
                response[U("error")] = json::value::string(U("Failed to close device"));
            }
        }
        else if (cmd == "loadTemplates")
        {
            if (!algorithmHandle_)
            {
                LOG_ERROR("错误: 算法未初始化");
                throw std::runtime_error("Algorithm not initialized");
            }

            LOG_INFO("开始加载模板到内存数据库...");
            try
            {
                auto templates = body.at(U("templates")).as_array();
                LOG_INFO("需要加载的模板数量: " + std::to_string(templates.size()));
                bool hasError = false;
                std::string errorMsg;

                // 先清空数据库
                int clearResult = FingerAlgorithm::clearTemplateDb(algorithmHandle_);
                if (clearResult != 1)
                {
                    LOG_ERROR("清空数据库失败");
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Failed to clear template database"));
                    return response;
                }
                LOG_INFO("清空数据库成功");

                for (const auto &templ : templates)
                {
                    int id = templ.at(U("id")).as_integer();
                    if (id <= 0)
                    {
                        hasError = true;
                        errorMsg = "Invalid template ID: " + std::to_string(id) + " (must be > 0)";
                        break;
                    }
                    LOG_INFO("正在加载模板 ID: " + std::to_string(id));

                    std::string templateData = utility::conversions::to_utf8string(templ.at(U("template")).as_string());
                    std::vector<unsigned char> templateBuffer = base64_decode(templateData);

                    if (templateBuffer.empty() || templateBuffer.size() > 2048)
                    { 
                        hasError = true;
                        errorMsg = "Invalid template data size for ID " + std::to_string(id);
                        break;
                    }

                    // 添加到数据库，返回值 >0 表示成功
                    int result = FingerAlgorithm::addTemplateToDb(
                        algorithmHandle_,
                        id,
                        templateBuffer.size(),
                        templateBuffer.data());

                    if (result <= 0)
                    {
                        hasError = true;
                        errorMsg = "Failed to load template for ID " + std::to_string(id);
                        break;
                    }
                    LOG_INFO("模板 " + std::to_string(id) + " 加载成功");
                }

                if (!hasError)
                {
                    // 获取已加载的模板数量
                    int count = FingerAlgorithm::getTemplateCount(algorithmHandle_);
                    LOG_INFO("模板加载完成，当前数据库中共有 " + std::to_string(count) + " 个模板");
                }

                response[U("success")] = json::value::boolean(!hasError);
                if (hasError)
                {
                    response[U("error")] = json::value::string(utility::conversions::to_string_t(errorMsg));
                    LOG_ERROR("加载失败: " + errorMsg);
                }
            }
            catch (const json::json_exception &e)
            {
                LOG_ERROR("请求参数错误: " + std::string(e.what()));
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Invalid request parameters"));
            }
        }
        else if (cmd == "capture")
        {
            if (!device_ || !algorithmHandle_)
            {
                LOG_ERROR("错误: 设备或算法未初始化");
                throw std::runtime_error("Device or algorithm not initialized");
            }

            int width = device_->getParameter(1);
            int height = device_->getParameter(2);

            std::vector<unsigned char> imageBuffer(width * height);
            int captureResult = device_->captureImage(imageBuffer.data(), imageBuffer.size());

            if (captureResult > 0)
            {
                std::vector<unsigned char> templateBuffer(2048); // 按文档建议分配 2048 字节
                int templateLength = FingerAlgorithm::extractTemplate(
                    algorithmHandle_,
                    imageBuffer.data(),
                    width,
                    height,
                    templateBuffer.data(),
                    templateBuffer.size(),
                    0 // flag 固定为 0
                );

                if (templateLength > 0)
                {
                    LOG_INFO("指纹采集和特征提取成功");
                    response[U("success")] = json::value::boolean(true);
                    response[U("template")] = json::value::string(
                        utility::conversions::to_string_t(base64_encode(templateBuffer)));
                    response[U("length")] = json::value::number(templateLength);
                }
                else
                {
                    LOG_ERROR("特征提取失败");
                    response[U("success")] = json::value::boolean(false);
                    response[U("error")] = json::value::string(U("Feature extraction failed"));
                }
            }
            else
            {
                LOG_ERROR("指纹采集失败");
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Capture failed"));
            }
        }
        else if (cmd == "verify")
        {
            if (!algorithmHandle_)
            {
                LOG_ERROR("错误: 算法未初始化");
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

            bool matched = score >= 50; // 按文档推荐阈值
            LOG_INFO("指纹比对完成，得分: " + std::to_string(score) + ", 结果: " + (matched ? "匹配" : "不匹配"));
            
            response[U("success")] = json::value::boolean(matched);
            response[U("score")] = json::value::number(score);
        }
        else if (cmd == "identify")
        {
            if (!algorithmHandle_)
            {
                LOG_ERROR("错误: 算法未初始化");
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
            response[U("threshold")] = json::value::number(70); // 文档推荐阈值

            if (result == 1) {
                LOG_INFO("指纹识别成功，匹配ID: " + std::to_string(matchedId) + ", 得分: " + std::to_string(score));
            } else {
                LOG_INFO("指纹识别失败，未找到匹配");
            }
        }
        else if (cmd == "register")
        {
            if (!algorithmHandle_)
            {
                LOG_ERROR("错误: 算法未初始化");
                throw std::runtime_error("Algorithm not initialized");
            }

            auto templateDataArray = body.at(U("templateData")).as_array();
            LOG_INFO("开始注册指纹，输入模板数量: " + std::to_string(templateDataArray.size()));

            std::vector<std::vector<unsigned char>> fingerTemplates;
            std::vector<unsigned char *> templatePtrs;

            for (const auto &templateData : templateDataArray)
            {
                auto templateStr = templateData.as_string();
                fingerTemplates.push_back(base64_decode(
                    utility::conversions::to_utf8string(templateStr)));
                templatePtrs.push_back(fingerTemplates.back().data());
            }

            std::vector<unsigned char> finalTemplate(2048); // 按文档建议分配 2048 字节
            int genResult = FingerAlgorithm::generateTemplate(
                algorithmHandle_,
                templatePtrs.data(),
                templatePtrs.size(),
                finalTemplate.data());

            if (genResult > 0)
            {
                LOG_INFO("指纹注册成功，生成最终模板");
                response[U("success")] = json::value::boolean(true);
                response[U("templateData")] = json::value::string(
                    utility::conversions::to_string_t(base64_encode(finalTemplate)));
            }
            else
            {
                LOG_ERROR("指纹注册失败，无法生成最终模板");
                response[U("success")] = json::value::boolean(false);
                response[U("error")] = json::value::string(U("Failed to generate template"));
            }
        }
        else if (cmd == "identify")
        {
            if (!algorithmHandle_)
            {
                LOG_ERROR("错误: 算法未初始化");
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
        else
        {
            LOG_ERROR("错误: 未知命令 " + cmd);
            throw std::runtime_error("Unknown command");
        }

        if (cmd != "capture")
        {
            std::string responseStr = response.serialize();
            LOG_INFO("命令执行完成，返回结果: " + responseStr);
        }
        return response;
    }
};

// 全局变量
std::condition_variable g_exit_cv;
std::mutex g_exit_mutex;

// 信号处理函数
void signal_handler(int)
{
    g_exit_cv.notify_one();
}

int main()
{
    FingerServer server;

    // 启动服务
    if (!server.start())
    {
        return 1;
    }

    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "\n按 Ctrl+C 退出...\n" << std::endl;

    // 等待退出信号
    std::unique_lock<std::mutex> lock(g_exit_mutex);
    g_exit_cv.wait(lock);

    // 停止服务
    server.stop();
    return 0;
}
