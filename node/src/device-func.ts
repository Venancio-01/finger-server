import { Library } from 'ffi-napi'
import { DEVICE_SDK_PATH } from './config'
import { DeviceArrayType, DeviceTypePointerType, HandleType, UcharType } from './types'

// 通过 ffi 解析 C++ SDK 方法
let fingerSensorSDK: any = null

export function initFingerSensorSDK() {
  fingerSensorSDK = Library(DEVICE_SDK_PATH, {
    sensorEnumDevices: ['int', [DeviceArrayType, 'int']], // 枚举设备
    sensorOpen: [HandleType, [DeviceTypePointerType]], // 打开设备
    sensorClose: ['int', [HandleType]], // 关闭设备
    sensorCapture: ['int', [HandleType, UcharType, 'int']], // 采集指纹
    sensorGetParameter: ['int', [HandleType, 'int']], // 获取指纹仪简单参数
  })
}

export function destroyFingerSensorSDK() {
  fingerSensorSDK = null
}

export function enumerateFingerSensors(sensorList: any, maxCount: number) {
  return fingerSensorSDK.sensorEnumDevices(sensorList, maxCount)
}

export function openFingerSensor(sensorHandle: unknown) {
  return fingerSensorSDK.sensorOpen(sensorHandle)
}

export function closeFingerSensor(sensorHandle: unknown) {
  return fingerSensorSDK.sensorClose(sensorHandle)
}

export function getFingerSensorParameter(sensorHandle: unknown, parameterType: 1 | 2) {
  return fingerSensorSDK.sensorGetParameter(sensorHandle, parameterType)
}

export function captureFingerprint(sensorHandle: unknown, fingerprintBuffer: unknown, bufferSize: number) {
  return fingerSensorSDK.sensorCapture(sensorHandle, fingerprintBuffer, bufferSize)
}
