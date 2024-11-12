#pragma once
#include <vector>

class FingerAlgorithm {
public:
    static bool initSDK();    // 初始化算法 SDK
    static void destroySDK(); // 销毁算法 SDK
    
    // 初始化算法
    // license: 固定为 0
    // width: 图像宽度
    // height: 图像高度
    // buffer: 可选参数，默认为 nullptr
    static void* initAlgorithm(int license, int width, int height, unsigned char* buffer = nullptr);
    
    // 关闭算法，返回值 1 表示成功
    static int closeAlgorithm(void* handle);
    
    // 添加模板到数据库，返回值 >0 表示成功
    // id: 模板ID(必须>0)
    static int addTemplateToDb(void* handle, int id, int length, unsigned char* data);
    
    // 从数据库删除模板，返回值 1 表示成功
    static int removeTemplateFromDb(void* handle, int id);
    
    // 清空数据库，返回值 1 表示成功
    static int clearTemplateDb(void* handle);
    
    // 1:N 识别，返回值 1 表示成功
    // score: 推荐阈值 70
    static int identifyTemplate(void* handle, unsigned char* data, int* id, int* score);
    
    // 1:1 验证，返回值��分数(0~100)
    // 推荐阈值 50
    static int verifyTemplate(void* handle, unsigned char* templ1, unsigned char* templ2);
    
    // 提取特征，返回值 >0 表示成功，返回模板长度
    // bufferSize: 建议 2048 字节
    // flag: 固定为 0
    static int extractTemplate(void* handle, unsigned char* image, int width, int height, 
                             unsigned char* templ, int bufferSize, int flag);
    
    // 生成注册模板，返回值 >0 表示成功，返回模板长度
    // count: 模板数量
    // output: 建议分配 2048 字节
    static int generateTemplate(void* handle, unsigned char** templates, int count, 
                              unsigned char* output);
    
    // 获取数据库中的模板数量
    static int getTemplateCount(void* handle);
    
private:
    static void* g_algorithmSDK;
}; 
