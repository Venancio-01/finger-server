#pragma once
#include <vector>
#include <string>

class FingerAlgorithm {
public:
    static bool initSDK();
    static void destroySDK();
    
    bool init(int width, int height);
    bool close();
    
    // 提取特征模板
    std::vector<unsigned char> extractTemplate(const std::vector<unsigned char>& image, int width, int height);
    
    // 生成注册模板
    std::vector<unsigned char> generateTemplate(const std::vector<std::vector<unsigned char>>& templates);
    
    // 1:N 识别
    bool identifyTemplate(const std::vector<unsigned char>& templ, int& matchedId, int& score);
    
    // 1:1 验证
    bool verifyTemplates(const std::vector<unsigned char>& templ1, const std::vector<unsigned char>& templ2);
    
    // 添加模板到数据库
    bool addTemplateToDb(int id, const std::vector<unsigned char>& templ);
    
private:
    void* algorithmHandle_ = nullptr;
}; 
