#pragma once

#include <string>
#include <functional>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

// 日志级别
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// 日志接口类
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, const std::string& message) = 0;
    virtual void close() = 0;
    
    // 获取全局日志实例
    static ILogger* getInstance() { return instance_; }
    
    // 设置全局日志实例
    static void setInstance(ILogger* logger) { instance_ = logger; }

private:
    static ILogger* instance_;
};

// 具体的日志实现类
class FileLogger : public ILogger {
public:
    FileLogger(const std::string& logPath = "/var/log/finger_server/") : logPath_(logPath) {
        initLogger();
    }
    
    ~FileLogger() {
        close();
    }
    
    void log(LogLevel level, const std::string& message) override {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << " [" << getLevelString(level) << "] " << message;
        
        // 同时输出到控制台和文件
        std::cout << ss.str() << std::endl;
        
        if (logFile_.is_open()) {
            logFile_ << ss.str() << std::endl;
            logFile_.flush();
        }
    }

    void close() override {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

private:
    std::string logPath_;
    std::ofstream logFile_;
    
    void initLogger() {
        try {
            // 确保日志目录存在
            std::filesystem::create_directories(logPath_);
            
            // 生成日志文件名（包含日期）
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << logPath_ << "finger_server_" 
               << std::put_time(std::localtime(&time), "%Y%m%d") << ".log";
               
            // 打开日志文件（追加模式）
            logFile_.open(ss.str(), std::ios::app);
            
            if (!logFile_.is_open()) {
                std::cerr << "无法打开日志文件: " << ss.str() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "初始化日志系统失败: " << e.what() << std::endl;
        }
    }
    
    const char* getLevelString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR:   return "ERROR";
            default:               return "UNKNOWN";
        }
    }
};

// 日志辅助宏
#define LOG_DEBUG(msg)   if(ILogger::getInstance()) ILogger::getInstance()->log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg)    if(ILogger::getInstance()) ILogger::getInstance()->log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) if(ILogger::getInstance()) ILogger::getInstance()->log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg)   if(ILogger::getInstance()) ILogger::getInstance()->log(LogLevel::ERROR, msg)
