#define ZKFP_DLOPEN
#include "libzkfp.h"

extern "C" {
    // 定义 SDK 函数
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
} 
