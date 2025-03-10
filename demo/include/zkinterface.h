#ifndef _ZKINTERFACE_H_
#define _ZKINTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define ZKINTERFACE __declspec(dllexport)
#define APICALL __stdcall
#else
#define ZKINTERFACE
#define APICALL
#endif

#ifdef ZKFP_DLOPEN
#define EXTERN_SDK_FUN extern
#else
#define EXTERN_SDK_FUN
#endif

#ifdef __cplusplus
}
#endif

#endif
