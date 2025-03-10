#include <cpprest/http_listener.h>
#include <cpprest/json.h>
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
#include <dlfcn.h>
#include <vector>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// 设备信息结构体
struct DeviceInfo {
    unsigned short vid;        // 设备厂商ID
    unsigned short pid;        // 设备产品ID
    unsigned char serialNumber[64];  // 设备序列号
    unsigned int busNumber;    // USB总线号
    unsigned int deviceAddress;  // 设备地址
    unsigned int extra;        // 扩展字段
};

// 设备 SDK 函数指针定义
typedef int (*SensorEnumDevices)(DeviceInfo*, int);
typedef void* (*SensorOpen)(DeviceInfo*);
typedef int (*SensorClose)(void*);
typedef int (*SensorCapture)(void*, unsigned char*, int);
typedef int (*SensorGetParameter)(void*, int);
typedef int (*SensorStatus)(void*);

// 算法 SDK 函数指针定义
typedef void* (*BiokeyInitSimple)(int, int, int, unsigned char*);
typedef int (*BiokeyClose)(void*);
typedef int (*BiokeyExtract)(void*, unsigned char*, unsigned char*, int);
typedef int (*BiokeyGetLastQuality)(void*);
typedef int (*BiokeyGenTemplate)(void*, unsigned char**, int, unsigned char*);
typedef int (*BiokeyVerify)(void*, unsigned char*, unsigned char*);
typedef int (*BiokeySetParameter)(void*, int, int);
typedef int (*BiokeyDbAdd)(void*, int, int, unsigned char*);
typedef int (*BiokeyDbDel)(void*, int);
typedef int (*BiokeyDbClear)(void*);
typedef int (*BiokeyDbCount)(void*);
typedef int (*BiokeyIdentifyTemp)(void*, unsigned char*, int*, int*);
typedef int (*BiokeyExtractGrayscaleData)(void*, unsigned char*, int, int, unsigned char*, int, int);

// 全局 SDK 句柄和函数指针
static void* g_deviceSDK = nullptr;
static void* g_algorithmSDK = nullptr;

// 设备 SDK 函数指针
static SensorEnumDevices g_enumDevices = nullptr;
static SensorOpen g_openDevice = nullptr;
static SensorClose g_closeDevice = nullptr;
static SensorCapture g_capture = nullptr;
static SensorGetParameter g_getParameter = nullptr;
static SensorStatus g_status = nullptr;

// 算法 SDK 函数指针
static BiokeyInitSimple g_initAlgorithm = nullptr;
static BiokeyClose g_closeAlgorithm = nullptr;
static BiokeyExtract g_extract = nullptr;
static BiokeyGetLastQuality g_getLastQuality = nullptr;
static BiokeyGenTemplate g_genTemplate = nullptr;
static BiokeyVerify g_verify = nullptr;
static BiokeySetParameter g_setParameter = nullptr;
static BiokeyDbAdd g_dbAdd = nullptr;
static BiokeyDbDel g_dbDel = nullptr;
static BiokeyDbClear g_dbClear = nullptr;
static BiokeyDbCount g_dbCount = nullptr;
static BiokeyIdentifyTemp g_identify = nullptr;
static BiokeyExtractGrayscaleData g_extractGrayscale = nullptr;

class FingerServer
{
public:
    FingerServer() : deviceHandle_(nullptr), algorithmHandle_(nullptr), isOpen_(false), logger_("/var/log/finger_server/")
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
            bool deviceInit = initDeviceSDK();
            bool algorithmInit = initAlgorithmSDK();

            if (deviceInit && algorithmInit)
            {
                LOG_INFO("SDK 初始化成功");
                return true;
            }
            else
            {
                LOG_ERROR("SDK 初始化失败");
                return false;
            }

            // 启动 HTTP 服务
            listener_.open().wait();
            LOG_INFO("指纹服务已启动，监听端口: 22813");
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
                closeAlgorithm(algorithmHandle_);
                algorithmHandle_ = nullptr;
            }
            if (deviceHandle_)
            {
                LOG_INFO("正在关闭设备...");
                closeDevice();
            }

            // 销毁 SDK
            LOG_INFO("正在销毁 SDK...");
            destroyDeviceSDK();
            destroyAlgorithmSDK();

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
    void* deviceHandle_;
    void* algorithmHandle_;
    bool isOpen_;
    std::vector<DeviceInfo> deviceList_;

    // 设备 SDK 相关函数
    bool initDeviceSDK() {
        g_deviceSDK = dlopen("libzkfpcap.so", RTLD_LAZY);
        if (!g_deviceSDK) {
            LOG_INFO("加载设备 SDK 失败: " + std::string(dlerror()));
            return false;
        }
            
        // 获取所有函数指针
        g_enumDevices = (SensorEnumDevices)dlsym(g_deviceSDK, "sensorEnumDevices");
        g_openDevice = (SensorOpen)dlsym(g_deviceSDK, "sensorOpen");
        g_closeDevice = (SensorClose)dlsym(g_deviceSDK, "sensorClose");
        g_capture = (SensorCapture)dlsym(g_deviceSDK, "sensorCapture");
        g_getParameter = (SensorGetParameter)dlsym(g_deviceSDK, "sensorGetParameter");
        g_status = (SensorStatus)dlsym(g_deviceSDK, "sensorStatus");
        
        // 检查所有函数是否加载成功
        if (!g_enumDevices || !g_openDevice || !g_closeDevice || 
            !g_capture || !g_getParameter || !g_status) {
            LOG_INFO("获取函数指针失败: " + std::string(dlerror()));
            dlclose(g_deviceSDK);
            g_deviceSDK = nullptr;
            return false;
        }
        
        return true;
    }

    void destroyDeviceSDK() {
        if (g_deviceSDK) {
            dlclose(g_deviceSDK);
            g_deviceSDK = nullptr;
            g_enumDevices = nullptr;
            g_openDevice = nullptr;
            g_closeDevice = nullptr;
            g_capture = nullptr;
            g_getParameter = nullptr;
            g_status = nullptr;
            LOG_INFO("设备 SDK 已销毁");
        }
    }

    bool isDeviceConnected() {
        deviceList_.resize(16);  // 文档建议的最大设备数
        int count = g_enumDevices(deviceList_.data(), deviceList_.size());
        LOG_INFO("检测到 " + std::to_string(count) + " 个设备");
        return count > 0;
    }

    bool openDevice() {
        if (isOpen_) {
            LOG_INFO("设备已经打开");
            return false;
        }
        
        if (deviceList_.empty()) {
            LOG_INFO("没有可用设备");
            return false;
        }
            
        deviceHandle_ = g_openDevice(&deviceList_[0]);
        if (!deviceHandle_) {
            LOG_INFO("打开设备失败");
            return false;
        }
        
        // 检查设备状态
        int status = g_status(deviceHandle_);
        if (status != 0) {
            LOG_INFO("设备状态异常: " + std::to_string(status));
            g_closeDevice(deviceHandle_);
            deviceHandle_ = nullptr;
            return false;
        }
            
        isOpen_ = true;
        LOG_INFO("设备打开成功");
        return true;
    }

    bool closeDevice() {
        if (!isOpen_) {
            LOG_INFO("设备未打开");
            return false;
        }
            
        int result = g_closeDevice(deviceHandle_);
        if (result == 0) {
            isOpen_ = false;
            deviceHandle_ = nullptr;
            LOG_INFO("设备关闭成功");
            return true;
        }
        
        LOG_INFO("设备关闭失败: " + std::to_string(result));
        return false;
    }

    int captureImage(unsigned char* buffer, int size) {
        if (!isOpen_) {
            LOG_INFO("设备未打开");
            return -1;
        }
        
        // 检查设备状态
        int status = g_status(deviceHandle_);
        if (status != 0) {
            LOG_INFO("设备状态异常: " + std::to_string(status));
            return -1;
        }
        
        int result = g_capture(deviceHandle_, buffer, size);
        if (result <= 0) {
            LOG_INFO("采集图像失败: " + std::to_string(result));
        }
        return result;
    }

    int getParameter(int type) {
        if (!isOpen_) {
            LOG_INFO("设备未打开");
            return -1;
        }
        
        // 根据文档，type 1 表示宽度，type 2 表示高度
        if (type != 1 && type != 2) {
            LOG_INFO("无效的参数类型: " + std::to_string(type));
            return -1;
        }
        
        int value = g_getParameter(deviceHandle_, type);
        if (value <= 0) {
            LOG_INFO("获取参数失败: type=" + std::to_string(type) + ", result=" + std::to_string(value));
        }
        return value;
    }

    // 算法 SDK 相关函数
    bool initAlgorithmSDK() {
        g_algorithmSDK = dlopen("libzkfp.so", RTLD_LAZY);
        if (!g_algorithmSDK) {
            LOG_ERROR("加载算法 SDK 失败: " + std::string(dlerror()));
            return false;
        }
        
        // 获取所有函数指针
        g_initAlgorithm = (BiokeyInitSimple)dlsym(g_algorithmSDK, "BIOKEY_INIT_SIMPLE");
        g_closeAlgorithm = (BiokeyClose)dlsym(g_algorithmSDK, "BIOKEY_CLOSE");
        g_extract = (BiokeyExtract)dlsym(g_algorithmSDK, "BIOKEY_EXTRACT");
        g_getLastQuality = (BiokeyGetLastQuality)dlsym(g_algorithmSDK, "BIOKEY_GETLASTQUALITY");
        g_genTemplate = (BiokeyGenTemplate)dlsym(g_algorithmSDK, "BIOKEY_GENTEMPLATE");
        g_verify = (BiokeyVerify)dlsym(g_algorithmSDK, "BIOKEY_VERIFY");
        g_setParameter = (BiokeySetParameter)dlsym(g_algorithmSDK, "BIOKEY_SET_PARAMETER");
        g_dbAdd = (BiokeyDbAdd)dlsym(g_algorithmSDK, "BIOKEY_DB_ADD");
        g_dbDel = (BiokeyDbDel)dlsym(g_algorithmSDK, "BIOKEY_DB_DEL");
        g_dbClear = (BiokeyDbClear)dlsym(g_algorithmSDK, "BIOKEY_DB_CLEAR");
        g_dbCount = (BiokeyDbCount)dlsym(g_algorithmSDK, "BIOKEY_DB_COUNT");
        g_identify = (BiokeyIdentifyTemp)dlsym(g_algorithmSDK, "BIOKEY_IDENTIFYTEMP");
        
        // 检查必要的函数是否加载成功
        if (!g_initAlgorithm || !g_closeAlgorithm || !g_extractGrayscale || 
            !g_genTemplate || !g_verify || !g_dbAdd || !g_identify) {
            LOG_ERROR("获取函数指针失败: " + std::string(dlerror()));
            dlclose(g_algorithmSDK);
            g_algorithmSDK = nullptr;
            return false;
        }
        
        LOG_INFO("算法 SDK 加载成功");
        return true;
    }

    void destroyAlgorithmSDK() {
        if (g_algorithmSDK) {
            dlclose(g_algorithmSDK);
            g_algorithmSDK = nullptr;
            LOG_INFO("算法 SDK 已销毁");
        }
    }

    void* initAlgorithm(int license, int width, int height, unsigned char* buffer = nullptr) {
        LOG_INFO("初始化算法 - 宽度: " + std::to_string(width) + ", 高度: " + std::to_string(height));
        void* handle = g_initAlgorithm(0, width, height, buffer);  // license 固定为 0
        if (!handle) {
            LOG_ERROR("算法初始化失败");
        } else {
            LOG_INFO("算法初始化成功");
        }
        return handle;
    }

    int closeAlgorithm(void* handle) {
        int result = g_closeAlgorithm(handle);
        LOG_INFO("关闭算法结果: " + std::string(result == 1 ? "成功" : "失败"));
        return result;  // 1 表示成功
    }

    int addTemplateToDb(void* handle, int id, int length, unsigned char* data) {
        LOG_INFO("添加模板到数据库 - ID: " + std::to_string(id) + ", 长度: " + std::to_string(length));
        int result = g_dbAdd(handle, id, length, data);
        if (result > 0) {
            LOG_INFO("模板添加成功");
        } else {
            LOG_ERROR("模板添加失败");
        }
        return result;  // >0 表示成功
    }

    int removeTemplateFromDb(void* handle, int id) {
        int result = g_dbDel(handle, id);
        LOG_INFO("删除模板结果: " + std::string(result == 1 ? "成功" : "失败"));
        return result;  // 1 表示成功
    }

    int clearTemplateDb(void* handle) {
        int result = g_dbClear(handle);
        LOG_INFO("清空数据库结果: " + std::string(result == 1 ? "成功" : "失败"));
        return result;  // 1 表示成功
    }

    int identifyTemplate(void* handle, unsigned char* data, int* id, int* score) {
        int result = g_identify(handle, data, id, score);
        if (result == 1) {
            LOG_INFO("指纹识别成功 - ID: " + std::to_string(*id) + ", 得分: " + std::to_string(*score));
        } else {
            LOG_INFO("指纹识别失败");
        }
        return result;  // 1 表示成功
    }

    int verifyTemplate(void* handle, unsigned char* templ1, unsigned char* templ2) {
        int score = g_verify(handle, templ1, templ2);
        LOG_INFO("指纹比对得分: " + std::to_string(score));
        return score;  // 返回分数(0~100)
    }

    int extractTemplate(void* handle, unsigned char* image, int width, int height, 
                       unsigned char* templ, int bufferSize, int flag) {
        int result = g_extract(handle, image, templ, flag);
        if (result > 0) {
            LOG_INFO("特征提取成功，模板长度: " + std::to_string(result));
        } else {
            LOG_ERROR("特征提取失败");
        }
        return result;  // >0 表示成功，返回模板长度
    }

    int generateTemplate(void* handle, unsigned char** templates, int count, unsigned char* output) {
        int result = g_genTemplate(handle, templates, count, output);
        if (result > 0) {
            LOG_INFO("生成注册模板成功，长度: " + std::to_string(result));
        } else {
            LOG_ERROR("生成注册模板失败");
        }
        return result;  // >0 表示成功，返回模板长度
    }

    int getTemplateCount(void* handle) {
        int count = g_dbCount(handle);
        LOG_INFO("当前数据库中的模板数量: " + std::to_string(count));
        return count;
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
            bool isConnected = isDeviceConnected();
            response[U("success")] = json::value::boolean(isConnected);
            LOG_INFO("设备连接状态: " + std::string(isConnected ? "已连接" : "未连接"));
        }
        else if (cmd == "openDevice")
        {
            if (!deviceHandle_)
            {
                LOG_ERROR("错误: 设备未初始化");
                throw std::runtime_error("Device not initialized");
            }

            bool success = openDevice();
            if (success)
            {
                int width = getParameter(1);  // 1=宽度
                int height = getParameter(2); // 2=高度
                LOG_INFO("设备参数 - 宽度: " + std::to_string(width) + ", 高度: " + std::to_string(height));

                algorithmHandle_ = initAlgorithm(0, width, height, nullptr);
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
            if (!deviceHandle_)
            {
                LOG_ERROR("错误: 设备未初始化");
                throw std::runtime_error("Device not initialized");
            }

            bool success = closeDevice();
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
                int clearResult = clearTemplateDb(algorithmHandle_);
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
                    int result = addTemplateToDb(
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
                    int count = getTemplateCount(algorithmHandle_);
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
            if (!deviceHandle_ || !algorithmHandle_)
            {
                LOG_ERROR("错误: 设备或算法未初始化");
                throw std::runtime_error("Device or algorithm not initialized");
            }

            int width = getParameter(1);
            int height = getParameter(2);

            std::vector<unsigned char> imageBuffer(width * height);
            int captureResult = captureImage(imageBuffer.data(), imageBuffer.size());

            if (captureResult > 0)
            {
                std::vector<unsigned char> templateBuffer(2048); // 按文档建议分配 2048 字节
                int templateLength = extractTemplate(
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

            int score = verifyTemplate(
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
            int result = identifyTemplate(
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
            int genResult = generateTemplate(
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
