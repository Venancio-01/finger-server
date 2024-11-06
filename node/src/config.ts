import { resolve } from 'path'

// finger libs 目录
export const fingerLibsPath = resolve(__dirname, '../lib')
// 指纹设备 SDK 路径
export const DEVICE_SDK_PATH = '/lib/libzkfpcap.so'
// 指纹算法 SDK 路径
export const ALGORITHM_SDK_PATH = '/lib/libzkfp.so'

// 设备最大连接数
export const MAX_DEVICE_NUM = 16
// 指纹登记次数
export const MAX_REGISTRATION_COUNT = 3
// 指纹模板字节数
export const TEMPLATE_BYTE_LENGTH = 2048
// 对比两个指纹后打分的阈值，低于 50 说明两个指纹不相同
export const VERIFY_SCORE_THRESHOLD = 50
