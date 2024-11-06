#include <napi.h>
#include "finger_device.h"

std::unique_ptr<FingerDevice> g_device;

Napi::Value InitSDK(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    bool success = FingerDevice::initSDK();
    if (success) {
        g_device = std::make_unique<FingerDevice>();
    }
    
    return Napi::Boolean::New(env, success);
}

Napi::Value IsDeviceConnected(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!g_device) {
        return Napi::Boolean::New(env, false);
    }
    
    return Napi::Boolean::New(env, g_device->isDeviceConnected());
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("initSDK", Napi::Function::New(env, InitSDK));
    exports.Set("isDeviceConnected", Napi::Function::New(env, IsDeviceConnected));
    return exports;
}

NODE_API_MODULE(finger, Init)
