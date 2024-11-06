#include "finger_algorithm.h"
#include <dlfcn.h>

// SDK 函数指针定义
typedef void* (*AlgorithmInit)(int, int, int, int);
typedef int (*AlgorithmClose)(void*);
typedef int (*ExtractTemplate)(void*, unsigned char*, int, int, unsigned char*, int, int);
typedef int (*IdentifyTemplate)(void*, unsigned char*, int*, int*);
typedef int (*GenerateTemplate)(void*, unsigned char**, int, unsigned char*);
typedef int (*VerifyTemplate)(void*, unsigned char*, unsigned char*);
typedef int (*AddTemplate)(void*, int, int, unsigned char*);

static void* g_algorithmSDK = nullptr;
static AlgorithmInit g_init = nullptr;
static AlgorithmClose g_close = nullptr;
static ExtractTemplate g_extract = nullptr;
static IdentifyTemplate g_identify = nullptr;
static GenerateTemplate g_generate = nullptr;
static VerifyTemplate g_verify = nullptr;
static AddTemplate g_addTemplate = nullptr;

bool FingerAlgorithm::initSDK() {
    g_algorithmSDK = dlopen("libzkfp.so", RTLD_LAZY);
    if (!g_algorithmSDK)
        return false;
        
    g_init = (AlgorithmInit)dlsym(g_algorithmSDK, "BIOKEY_INIT_SIMPLE");
    g_close = (AlgorithmClose)dlsym(g_algorithmSDK, "BIOKEY_CLOSE");
    g_extract = (ExtractTemplate)dlsym(g_algorithmSDK, "BIOKEY_EXTRACT_GRAYSCALEDATA");
    g_identify = (IdentifyTemplate)dlsym(g_algorithmSDK, "BIOKEY_IDENTIFYTEMP");
    g_generate = (GenerateTemplate)dlsym(g_algorithmSDK, "BIOKEY_GENTEMPLATE");
    g_verify = (VerifyTemplate)dlsym(g_algorithmSDK, "BIOKEY_VERIFY");
    g_addTemplate = (AddTemplate)dlsym(g_algorithmSDK, "BIOKEY_DB_ADD");
    
    return true;
} 
