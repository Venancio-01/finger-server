#pragma once
#include <vector>

class FingerAlgorithm {
public:
    static bool initSDK();
    static void destroySDK();
    
    // 初始化算法，dpi 参数可以为 nullptr
    static void* initAlgorithm(int mode, int width, int height, void* dpi = nullptr);
    
    // 关闭算法
    static int closeAlgorithm(void* handle);
    
    // 添加模板到数据库
    static int addTemplateToDb(void* handle, int id, int length, unsigned char* data);
    
    // 从数据库删除模板
    static int removeTemplateFromDb(void* handle, int id);
    
    // 清空数据库
    static int clearTemplateDb(void* handle);
    
    // 1:N 识别
    static int identifyTemplate(void* handle, unsigned char* data, int* id, int* score);
    
    // 1:1 验证
    static int verifyTemplate(void* handle, unsigned char* templ1, unsigned char* templ2);
    
    // 提取特征
    static int extractTemplate(void* handle, unsigned char* image, int width, int height, 
                             unsigned char* templ, int bufferSize, int flag);
    
    // 生成注册模板
    static int generateTemplate(void* handle, unsigned char** templates, int count, 
                              unsigned char* output);
    
private:
    static void* g_algorithmSDK;
    static void* getFunction(const char* name);
}; 
