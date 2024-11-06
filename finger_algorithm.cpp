#include "finger_algorithm.h"
#include <dlfcn.h>

void* FingerAlgorithm::g_algorithmSDK = nullptr;

bool FingerAlgorithm::initSDK() {
    g_algorithmSDK = dlopen("libzkfp.so", RTLD_LAZY);
    if (!g_algorithmSDK)
        return false;
    
    return true;
}

void FingerAlgorithm::destroySDK() {
    if (g_algorithmSDK) {
        dlclose(g_algorithmSDK);
        g_algorithmSDK = nullptr;
    }
}

void* FingerAlgorithm::getFunction(const char* name) {
    if (!g_algorithmSDK)
        return nullptr;
    return dlsym(g_algorithmSDK, name);
}

void* FingerAlgorithm::initAlgorithm(int mode, int width, int height, int dpi) {
    typedef void* (*Func)(int, int, int, int);
    auto func = (Func)getFunction("BIOKEY_INIT_SIMPLE");
    if (!func) return nullptr;
    return func(mode, width, height, dpi);
}

int FingerAlgorithm::closeAlgorithm(void* handle) {
    typedef int (*Func)(void*);
    auto func = (Func)getFunction("BIOKEY_CLOSE");
    if (!func) return -1;
    return func(handle);
}

int FingerAlgorithm::addTemplateToDb(void* handle, int id, int length, unsigned char* data) {
    typedef int (*Func)(void*, int, int, unsigned char*);
    auto func = (Func)getFunction("BIOKEY_DB_ADD");
    if (!func) return -1;
    return func(handle, id, length, data);
}

int FingerAlgorithm::removeTemplateFromDb(void* handle, int id) {
    typedef int (*Func)(void*, int);
    auto func = (Func)getFunction("BIOKEY_DB_DEL");
    if (!func) return -1;
    return func(handle, id);
}

int FingerAlgorithm::clearTemplateDb(void* handle) {
    typedef int (*Func)(void*);
    auto func = (Func)getFunction("BIOKEY_DB_CLEAR");
    if (!func) return -1;
    return func(handle);
}

int FingerAlgorithm::identifyTemplate(void* handle, unsigned char* data, int* id, int* score) {
    typedef int (*Func)(void*, unsigned char*, int*, int*);
    auto func = (Func)getFunction("BIOKEY_IDENTIFYTEMP");
    if (!func) return -1;
    return func(handle, data, id, score);
}

int FingerAlgorithm::verifyTemplate(void* handle, unsigned char* templ1, unsigned char* templ2) {
    typedef int (*Func)(void*, unsigned char*, unsigned char*);
    auto func = (Func)getFunction("BIOKEY_VERIFY");
    if (!func) return -1;
    return func(handle, templ1, templ2);
}

int FingerAlgorithm::extractTemplate(void* handle, unsigned char* image, int width, int height, 
                                   unsigned char* templ, int bufferSize, int flag) {
    typedef int (*Func)(void*, unsigned char*, int, int, unsigned char*, int, int);
    auto func = (Func)getFunction("BIOKEY_EXTRACT_GRAYSCALEDATA");
    if (!func) return -1;
    return func(handle, image, width, height, templ, bufferSize, flag);
}

int FingerAlgorithm::generateTemplate(void* handle, unsigned char** templates, int count, 
                                    unsigned char* output) {
    typedef int (*Func)(void*, unsigned char**, int, unsigned char*);
    auto func = (Func)getFunction("BIOKEY_GENTEMPLATE");
    if (!func) return -1;
    return func(handle, templates, count, output);
}
