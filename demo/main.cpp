#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/sysinfo.h>
#include "include/zkinterface.h"
#include "include/libzkfperrdef.h"
#include "include/libzkfptype.h"
#include "include/libzkfp.h"
#include "include/tool.h"

#define LogDebug (printf("[%d][%s]",__LINE__,__FILE__),printf)

#ifndef HANDLE
#define HANDLE void *
#endif

#define ENROLLCNT 3

// 全局变量
HANDLE g_libHandle = NULL;
HANDLE g_hDevice = NULL;
HANDLE g_hDBCache = NULL;
unsigned char *g_pImgBuf = NULL;
unsigned char g_arrPreRegTemps[ENROLLCNT][MAX_TEMPLATE_SIZE];
unsigned int g_arrPreTempsLen[3];
int g_nLastRegTempLen = 0;
unsigned char g_szLastRegTemplate[MAX_TEMPLATE_SIZE];
bool g_bIdentify = false;
bool g_bRegister = false;
int g_enrollIdx = 0;

// SDK函数指针
T_ZKFPM_Init ZKFPM_Init = nullptr;
T_ZKFPM_Terminate ZKFPM_Terminate = nullptr;
T_ZKFPM_GetDeviceCount ZKFPM_GetDeviceCount = nullptr;
T_ZKFPM_OpenDevice ZKFPM_OpenDevice = nullptr;
T_ZKFPM_CloseDevice ZKFPM_CloseDevice = nullptr;
T_ZKFPM_SetParameters ZKFPM_SetParameters = nullptr;
T_ZKFPM_GetParameters ZKFPM_GetParameters = nullptr;
T_ZKFPM_AcquireFingerprint ZKFPM_AcquireFingerprint = nullptr;
T_ZKFPM_AcquireFingerprintImage ZKFPM_AcquireFingerprintImage = nullptr;
T_ZKFPM_DBInit ZKFPM_DBInit = nullptr;
T_ZKFPM_DBFree ZKFPM_DBFree = nullptr;
T_ZKFPM_DBMerge ZKFPM_DBMerge = nullptr;
T_ZKFPM_DBAdd ZKFPM_DBAdd = nullptr;
T_ZKFPM_DBDel ZKFPM_DBDel = nullptr;
T_ZKFPM_DBClear ZKFPM_DBClear = nullptr;
T_ZKFPM_DBCount ZKFPM_DBCount = nullptr;
T_ZKFPM_DBIdentify ZKFPM_DBIdentify = nullptr;
T_ZKFPM_DBMatch ZKFPM_DBMatch = nullptr;
T_ZKFPM_ExtractFromImage ZKFPM_ExtractFromImage = nullptr;
T_ZKFPM_SetLogLevel ZKFPM_SetLogLevel = nullptr;
T_ZKFPM_ConfigLog ZKFPM_ConfigLog = nullptr;

// 工具函数
static void *LoadSym(void *pHandle, const char *pSymbol) {
    void *pAPI = NULL;
    char *pErrMsg = NULL;

    pAPI = dlsym(pHandle, pSymbol);
    pErrMsg = dlerror();
    if (NULL != pErrMsg) {
        LogDebug("Load function error: %s\n", pErrMsg);
        return NULL;
    }
    return pAPI;
}

// 加载动态库
bool LoadLib() {
    g_libHandle = dlopen("libzkfp.so", RTLD_NOW);
    if (NULL == g_libHandle) {
        LogDebug("Load libzkfp failed, error: %s\n", dlerror());
        return false;
    }

    // 加载所有函数
    ZKFPM_Init = (T_ZKFPM_Init)LoadSym(g_libHandle, "ZKFPM_Init");
    ZKFPM_Terminate = (T_ZKFPM_Terminate)LoadSym(g_libHandle, "ZKFPM_Terminate");
    ZKFPM_GetDeviceCount = (T_ZKFPM_GetDeviceCount)LoadSym(g_libHandle, "ZKFPM_GetDeviceCount");
    ZKFPM_OpenDevice = (T_ZKFPM_OpenDevice)LoadSym(g_libHandle, "ZKFPM_OpenDevice");
    ZKFPM_CloseDevice = (T_ZKFPM_CloseDevice)LoadSym(g_libHandle, "ZKFPM_CloseDevice");
    ZKFPM_SetParameters = (T_ZKFPM_SetParameters)LoadSym(g_libHandle, "ZKFPM_SetParameters");
    ZKFPM_GetParameters = (T_ZKFPM_GetParameters)LoadSym(g_libHandle, "ZKFPM_GetParameters");
    ZKFPM_AcquireFingerprint = (T_ZKFPM_AcquireFingerprint)LoadSym(g_libHandle, "ZKFPM_AcquireFingerprint");
    ZKFPM_AcquireFingerprintImage = (T_ZKFPM_AcquireFingerprintImage)LoadSym(g_libHandle, "ZKFPM_AcquireFingerprintImage");
    ZKFPM_DBInit = (T_ZKFPM_DBInit)LoadSym(g_libHandle, "ZKFPM_DBInit");
    ZKFPM_DBFree = (T_ZKFPM_DBFree)LoadSym(g_libHandle, "ZKFPM_DBFree");
    ZKFPM_DBMerge = (T_ZKFPM_DBMerge)LoadSym(g_libHandle, "ZKFPM_DBMerge");
    ZKFPM_DBAdd = (T_ZKFPM_DBAdd)LoadSym(g_libHandle, "ZKFPM_DBAdd");
    ZKFPM_DBDel = (T_ZKFPM_DBDel)LoadSym(g_libHandle, "ZKFPM_DBDel");
    ZKFPM_DBClear = (T_ZKFPM_DBClear)LoadSym(g_libHandle, "ZKFPM_DBClear");
    ZKFPM_DBCount = (T_ZKFPM_DBCount)LoadSym(g_libHandle, "ZKFPM_DBCount");
    ZKFPM_DBIdentify = (T_ZKFPM_DBIdentify)LoadSym(g_libHandle, "ZKFPM_DBIdentify");
    ZKFPM_DBMatch = (T_ZKFPM_DBMatch)LoadSym(g_libHandle, "ZKFPM_DBMatch");
    ZKFPM_ExtractFromImage = (T_ZKFPM_ExtractFromImage)LoadSym(g_libHandle, "ZKFPM_ExtractFromImage");
    ZKFPM_SetLogLevel = (T_ZKFPM_SetLogLevel)LoadSym(g_libHandle, "ZKFPM_SetLogLevel");
    ZKFPM_ConfigLog = (T_ZKFPM_ConfigLog)LoadSym(g_libHandle, "ZKFPM_ConfigLog");

    return true;
}

// 初始化设备
bool InitDevice() {
    if (ZKFPM_Init() != ZKFP_ERR_OK) {
        LogDebug("Init ZKFPM failed\n");
        return false;
    }

    if (ZKFPM_GetDeviceCount() <= 0) {
        LogDebug("No device found\n");
        ZKFPM_Terminate();
        return false;
    }

    g_hDevice = ZKFPM_OpenDevice(0);
    if (g_hDevice == NULL) {
        LogDebug("Open device failed\n");
        ZKFPM_Terminate();
        return false;
    }

    g_hDBCache = ZKFPM_DBInit();
    if (g_hDBCache == NULL) {
        LogDebug("Create DBCache failed\n");
        ZKFPM_CloseDevice(g_hDevice);
        ZKFPM_Terminate();
        return false;
    }

    return true;
}

// 获取设备参数
bool GetDeviceParameters(int& width, int& height) {
    unsigned char paramValue[4] = {0};
    unsigned int cbParamValue = 4;

    // 获取宽度
    if (ZKFPM_GetParameters(g_hDevice, 1, paramValue, &cbParamValue) != ZKFP_ERR_OK) {
        return false;
    }
    width = *(int*)paramValue;

    // 获取高度
    memset(paramValue, 0, 4);
    cbParamValue = 4;
    if (ZKFPM_GetParameters(g_hDevice, 2, paramValue, &cbParamValue) != ZKFP_ERR_OK) {
        return false;
    }
    height = *(int*)paramValue;

    return true;
}

// 验证指纹
void DoVerify(unsigned char* temp, int len) {
    if (g_nLastRegTempLen <= 0) {
        LogDebug("Please register first\n");
        return;
    }

    if (g_bIdentify) {
        unsigned int tid = 0;
        unsigned int score = 0;
        int ret = ZKFPM_DBIdentify(g_hDBCache, temp, len, &tid, &score);
        if (ret != ZKFP_ERR_OK) {
            LogDebug("Identify failed, ret = %d\n", ret);
        } else {
            LogDebug("Identify success, tid=%d, score=%d\n", tid, score);
        }
    } else {
        int score = ZKFPM_DBMatch(g_hDBCache, g_szLastRegTemplate, g_nLastRegTempLen, temp, len);
        if (score <= ZKFP_ERR_OK) {
            LogDebug("Verify failed, score=%d\n", score);
        } else {
            LogDebug("Verify success, score=%d\n", score);
        }
    }
}

// 注册指纹
int DoRegister(unsigned char* temp, int len) {
    if (g_enrollIdx >= ENROLLCNT) {
        g_enrollIdx = 0;
        return 1;
    }

    if (g_enrollIdx > 0) {
        int ret = ZKFPM_DBMatch(g_hDBCache, g_arrPreRegTemps[g_enrollIdx-1], 
                               g_arrPreTempsLen[g_enrollIdx-1], temp, len);
        if (ret <= ZKFP_ERR_OK) {
            g_enrollIdx = 0;
            LogDebug("Register failed, please place the same finger\n");
            return 1;
        }
    }

    g_arrPreTempsLen[g_enrollIdx] = len;
    memcpy(g_arrPreRegTemps[g_enrollIdx], temp, len);

    if (++g_enrollIdx >= ENROLLCNT) {
        unsigned char regTemp[MAX_TEMPLATE_SIZE];
        unsigned int cbRegTemp = MAX_TEMPLATE_SIZE;
        int ret = ZKFPM_DBMerge(g_hDBCache, g_arrPreRegTemps[0], g_arrPreRegTemps[1], 
                               g_arrPreRegTemps[2], regTemp, &cbRegTemp);
        if (ret == ZKFP_ERR_OK) {
            g_nLastRegTempLen = cbRegTemp;
            memcpy(g_szLastRegTemplate, regTemp, cbRegTemp);
            LogDebug("Register success\n");
        } else {
            g_enrollIdx = 0;
            LogDebug("Register failed, error=%d\n", ret);
        }
        return 1;
    }

    LogDebug("Please press the same finger %d times\n", ENROLLCNT - g_enrollIdx);
    return 0;
}

// 清理资源
void Cleanup() {
    if (g_pImgBuf) {
        free(g_pImgBuf);
        g_pImgBuf = NULL;
    }
    if (g_hDBCache) {
        ZKFPM_DBFree(g_hDBCache);
        g_hDBCache = NULL;
    }
    if (g_hDevice) {
        ZKFPM_CloseDevice(g_hDevice);
        g_hDevice = NULL;
    }
    ZKFPM_Terminate();
    if (g_libHandle) {
        dlclose(g_libHandle);
        g_libHandle = NULL;
    }
}

// 信号处理
static void sighandler(int signo) {
    Cleanup();
    exit(signo);
}

void SetExitSignalHandle() {
    struct sigaction sa;
    int sigs[] = {
        SIGILL, SIGFPE, SIGABRT, SIGBUS,
        SIGSEGV, SIGHUP, SIGINT, SIGQUIT,
        SIGTERM
    };

    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    
    for (size_t i = 0; i < sizeof(sigs)/sizeof(sigs[0]); ++i) {
        if (sigaction(sigs[i], &sa, NULL) == -1) {
            perror("Could not set signal handler");
        }
    }
}

int main(int argc, char *argv[]) {
    // 加载动态库
    if (!LoadLib()) {
        return 1;
    }

    // 设置信号处理
    SetExitSignalHandle();

    // 初始化设备
    if (!InitDevice()) {
        Cleanup();
        return 1;
    }

    // 获取设备参数
    int width = 0, height = 0;
    if (!GetDeviceParameters(width, height)) {
        LogDebug("Get device parameters failed\n");
        Cleanup();
        return 1;
    }

    // 分配图像缓冲区
    int imageBufferSize = width * height;
    g_pImgBuf = (unsigned char*)malloc(imageBufferSize);
    if (g_pImgBuf == NULL) {
        LogDebug("Malloc image buffer failed\n");
        Cleanup();
        return 1;
    }

    LogDebug("Device initialized. Width=%d, Height=%d\n", width, height);

    // 主循环
    char cmd;
    while (true) {
        LogDebug("\nPlease select operation:\n");
        LogDebug("1. Register\n");
        LogDebug("2. Verify\n");
        LogDebug("3. Identify\n");
        LogDebug("4. Quit\n");
        
        cmd = getchar();
        getchar(); // 消费换行符

        switch (cmd) {
            case '1':
                g_bRegister = true;
                g_bIdentify = false;
                g_enrollIdx = 0;
                break;
            case '2':
                g_bRegister = false;
                g_bIdentify = false;
                break;
            case '3':
                g_bRegister = false;
                g_bIdentify = true;
                break;
            case '4':
                Cleanup();
                return 0;
            default:
                continue;
        }

        // 采集和处理指纹
        while (true) {
            unsigned char template_data[MAX_TEMPLATE_SIZE];
            unsigned int template_size = MAX_TEMPLATE_SIZE;

            // 采集指纹
            int ret = ZKFPM_AcquireFingerprint(g_hDevice, g_pImgBuf, imageBufferSize,
                                              template_data, &template_size);
            
            if (ret == ZKFP_ERR_OK) {
                if (g_bRegister) {
                    if (DoRegister(template_data, template_size)) {
                        break;
                    }
                } else {
                    DoVerify(template_data, template_size);
                    break;
                }
            } else {
                LogDebug("Acquire fingerprint failed, ret=%d\n", ret);
            }
            
            usleep(100000);
        }
    }

    Cleanup();
    return 0;
}
