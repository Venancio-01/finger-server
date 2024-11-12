#include "finger_algorithm.h"
#include <dlfcn.h>
#include <iostream>

void* FingerAlgorithm::g_algorithmSDK = nullptr;

// SDK 函数指针定义
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

bool FingerAlgorithm::initSDK() {
    g_algorithmSDK = dlopen("libzkfp.so", RTLD_LAZY);
    if (!g_algorithmSDK) {
        std::cout << "加载算法 SDK 失败: " << dlerror() << std::endl;
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
    g_extractGrayscale = (BiokeyExtractGrayscaleData)dlsym(g_algorithmSDK, "BIOKEY_EXTRACT_GRAYSCALEDATA");
    
    // 检查必要的函数是否加载成功
    if (!g_initAlgorithm || !g_closeAlgorithm || !g_extractGrayscale || 
        !g_genTemplate || !g_verify || !g_dbAdd || !g_identify) {
        std::cout << "获取函数指针失败: " << dlerror() << std::endl;
        dlclose(g_algorithmSDK);
        g_algorithmSDK = nullptr;
        return false;
    }
    
    return true;
}

void FingerAlgorithm::destroySDK() {
    if (g_algorithmSDK) {
        dlclose(g_algorithmSDK);
        g_algorithmSDK = nullptr;
    }
}

void* FingerAlgorithm::initAlgorithm(int license, int width, int height, unsigned char* buffer) {
    std::cout << "初始化算法 - 宽度: " << width << ", 高度: " << height << std::endl;
    void* handle = g_initAlgorithm(0, width, height, buffer);  // license 固定为 0
    if (!handle) {
        std::cout << "算法初始化失败" << std::endl;
    }
    return handle;
}

int FingerAlgorithm::closeAlgorithm(void* handle) {
    int result = g_closeAlgorithm(handle);
    std::cout << "关闭算法结果: " << (result == 1 ? "成功" : "失败") << std::endl;
    return result;  // 1 表示成功
}

int FingerAlgorithm::addTemplateToDb(void* handle, int id, int length, unsigned char* data) {
    std::cout << "添加模板到数据库 - ID: " << id << ", 长度: " << length << std::endl;
    int result = g_dbAdd(handle, id, length, data);
    std::cout << "添加结果: " << (result > 0 ? "成功" : "失败") << std::endl;
    return result;  // >0 表示成功
}

int FingerAlgorithm::removeTemplateFromDb(void* handle, int id) {
    int result = g_dbDel(handle, id);
    std::cout << "删除模板结果: " << (result == 1 ? "成功" : "失败") << std::endl;
    return result;  // 1 表示成功
}

int FingerAlgorithm::clearTemplateDb(void* handle) {
    int result = g_dbClear(handle);
    std::cout << "清空数据库结果: " << (result == 1 ? "成功" : "失败") << std::endl;
    return result;  // 1 表示成功
}

int FingerAlgorithm::identifyTemplate(void* handle, unsigned char* data, int* id, int* score) {
    int result = g_identify(handle, data, id, score);
    std::cout << "识别结果: " << (result == 1 ? "成功" : "失败") 
              << ", 匹配ID: " << *id 
              << ", 分数: " << *score << std::endl;
    return result;  // 1 表示成功
}

int FingerAlgorithm::verifyTemplate(void* handle, unsigned char* templ1, unsigned char* templ2) {
    int score = g_verify(handle, templ1, templ2);
    std::cout << "比对分数: " << score << " (推荐阈值: 50)" << std::endl;
    return score;  // 返回分数(0~1000)
}

int FingerAlgorithm::extractTemplate(void* handle, unsigned char* image, int width, int height, 
                                   unsigned char* templ, int bufferSize, int flag) {
    int result = g_extractGrayscale(handle, image, width, height, templ, bufferSize, flag);
    if (result > 0) {
        int quality = g_getLastQuality(handle);
        std::cout << "特征提取成功 - 长度: " << result << ", 质量: " << quality << std::endl;
    } else {
        std::cout << "特征提取失败" << std::endl;
    }
    return result;  // >0 表示成功，返回模板长度
}

int FingerAlgorithm::generateTemplate(void* handle, unsigned char** templates, int count, 
                                    unsigned char* output) {
    int result = g_genTemplate(handle, templates, count, output);
    std::cout << "生成注册模板结果: " << (result > 0 ? "成功" : "失败") << std::endl;
    return result;  // >0 表示成功，返回模板长度
}

int FingerAlgorithm::getTemplateCount(void* handle) {
    int count = g_dbCount(handle);
    std::cout << "当前数据库模板数量: " << count << std::endl;
    return count;
}
