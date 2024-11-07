import { existsSync } from 'fs'
import { execSync } from 'child_process'
import { Library } from 'ffi-napi'
import { HandleType, IntType, TemplateType, UcharType } from './types'
import { ALGORITHM_SDK_PATH, VERIFY_SCORE_THRESHOLD, fingerLibsPath } from './config'

// 检查指纹库文件是否存在
export function isFingerLibExists() {
  const libPath = '/lib/libzkfinger10.so'
  const exists = existsSync(libPath)

  if (exists) {
    return true
  }

  try {
    execSync(`sudo cp ${fingerLibsPath}/* /lib`)
    console.log('指纹 SDK 文件复制成功')
    return true
  }
  catch (err: any) {
    console.error(`指纹 SDK 文件复制失败: ${err.message}`)
    return false
  }
}

let fingerAlgorithmSDK: any = null

// 通过 ffi 解析 C++ SDK 方法
export function initFingerAlgorithmSDK() {
  fingerAlgorithmSDK = Library(ALGORITHM_SDK_PATH, {
    BIOKEY_INIT_SIMPLE: [HandleType, ['int', 'int', 'int', 'int']], // 初始化算法
    BIOKEY_CLOSE: ['int', [HandleType]], // 释放算法
    BIOKEY_EXTRACT_GRAYSCALEDATA: ['int', [HandleType, UcharType, 'int', 'int', UcharType, 'int', 'int']], // 提取模版
    BIOKEY_IDENTIFYTEMP: ['int', [HandleType, UcharType, IntType, IntType]], // 1:N 识别
    BIOKEY_GENTEMPLATE: ['int', [HandleType, TemplateType, 'int', UcharType]], // ⽣成登记特征(多个模板之中取最好)
    BIOKEY_VERIFY: ['int', [HandleType, UcharType, UcharType]], // 对比两个模板
    BIOKEY_DB_ADD: ['int', [HandleType, 'int', 'int', UcharType]], // 添加模板到1:N内存中
    BIOKEY_DB_DEL: ['int', [HandleType, 'int']], // 从内存中删除⼀个模板
    BIOKEY_DB_CLEAR: ['int', [HandleType]], // 清空内存中全部模板
  })
}

export function destroyFingerAlgorithmSDK() {
  fingerAlgorithmSDK = null
}

/**
 * @description: 初始化算法
 * @param {number} width 设备宽度
 * @param {number} height 设备高度
 * @return {*} 返回算法句柄
 */
export function initFingerAlgorithm(width: number, height: number) {
  return fingerAlgorithmSDK.BIOKEY_INIT_SIMPLE(0, width, height, 0)
}

/**
 * @description: 释放算法
 * @param {unknown} handle 算法句柄
 * @return {*} 1 表⽰成功,其余表⽰失败
 */
export function closeFingerAlgorithm(handle: unknown): number {
  return fingerAlgorithmSDK.BIOKEY_CLOSE(handle)
}

/**
 * @description: 生成注册的指纹模板
 * @param {unknown} handle 算法句柄
 * @param {unknown} templates 模板数组
 * @param {number} num 数组个数
 * @param {unknown} gTemplate 返回最好的模板(建议分配2048字节)
 * @return {*} >0 表⽰成功，值为最好模板的实际数据⻓度
 */
export function generateFingerTemplate(handle: unknown, templates: unknown, num: number, gTemplate: unknown) {
  const result = fingerAlgorithmSDK.BIOKEY_GENTEMPLATE(handle, templates, num, gTemplate)
  const success = result > 0
  return {
    success,
    result,
  }
}

/**
 * @description: 对比两个模板
 * @param {*} handle 算法句柄
 * @param {*} template1 模板一
 * @param {*} template2 模板二
 * @return {boolean} 返回比对成功的结果
 */
export function verifyFingerTemplates(handle, template1, template2): boolean {
  // 返回分数(0~1000), 推荐阈值50
  const score = fingerAlgorithmSDK.BIOKEY_VERIFY(handle, template1, template2)
  return score >= VERIFY_SCORE_THRESHOLD
}

/**
 * @description: 指纹 1:N 比对
 * @param {unknown} handle 算法句柄
 * @param {unknown} templateDate 模版数据
 * @param {unknown} tid 返回识别成功的指纹ID
 * @param {unknown} score 返回识别成功的分数(推荐阈值70)
 * @return {number} 成功返回1
 */
export function identifyFingerTemplate(handle: unknown, templateDate: unknown, tid: unknown, score: unknown): number {
  return fingerAlgorithmSDK.BIOKEY_IDENTIFYTEMP(handle, templateDate, tid, score)
}

/**
 * @description: 提取模版
 * @param {unknown} handle 算法句柄
 * @param {unknown} imageBuffer sensorCapture 提取的图像数据
 * @param {number} width 图像宽度
 * @param {number} height 图像高度
 * @param {unknown} template 返回指纹模版数据（建议 2048 字节）
 * @param {number} len
 * @return {number} > 0 表⽰提取成功，返回模板数据实际⻓度
 */
export function extractFingerTemplate(
  handle: unknown,
  imageBuffer: unknown,
  width: number,
  height: number,
  template: unknown,
  len: number,
): number {
  return fingerAlgorithmSDK.BIOKEY_EXTRACT_GRAYSCALEDATA(handle, imageBuffer, width, height, template, len, 0)
}

/**
 * @description: 添加模板到1:N内存中
 * @param {unknown} handle 算法句柄
 * @param {number} tid 指纹id
 * @param {number} templateLength 指纹模板数据长度
 * @param {unknown} templateData 指纹模板数据
 * @return {*} 返回添加结果和代码
 */
export function addFingerTemplateToDb(handle: unknown, tid: number, templateLength: number, templateData: unknown) {
  const result = fingerAlgorithmSDK.BIOKEY_DB_ADD(handle, tid, templateLength, templateData)
  const success = result === 1
  return { success, result }
}

/**
 * @description: 从内存中删除⼀个模板
 * @param {unknown} handle 算法句柄
 * @param {number} tid 指纹id
 * @return {*} 返回添加结果和代码
 */
export function removeFingerTemplateById(handle: unknown, tid: number) {
  const result = fingerAlgorithmSDK.BIOKEY_DB_DEL(handle, tid)
  const success = result === 1
  return { success, result }
}

/**
 * @description: 清空内存中全部模板
 * @param {unknown} handle 算法句柄
 * @return {*} 返回添加结果和代码
 */
export function clearFingerTemplateDb(handle: unknown) {
  const result = fingerAlgorithmSDK.BIOKEY_DB_CLEAR(handle)
  const success = result === 1
  return { success, result }
}
