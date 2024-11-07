import { genResponseData } from './utils'
import { MAX_DEVICE_NUM, MAX_REGISTRATION_COUNT, TEMPLATE_BYTE_LENGTH } from './config'
import {
  captureFingerprint,
  closeFingerSensor,
  destroyFingerSensorSDK,
  enumerateFingerSensors,
  getFingerSensorParameter,
  initFingerSensorSDK,
  openFingerSensor,
} from './device-func'
import {
  addFingerTemplateToDb,
  closeFingerAlgorithm,
  destroyFingerAlgorithmSDK,
  extractFingerTemplate,
  generateFingerTemplate,
  identifyFingerTemplate,
  initFingerAlgorithm,
  initFingerAlgorithmSDK,
  isFingerLibExists,
  verifyFingerTemplates,
} from './algorithm-func'
import { DeviceArrayType, IntType, TemplateType, UcharType } from './types'

// 指纹仪设备数组
const deviceList = new DeviceArrayType(MAX_DEVICE_NUM)
// 当前指纹仪在线状态
let connected = false
// 当前指纹仪开启状态
let isOpen = false
// 当前接入的指纹仪设备句柄
let deviceHandle = null
// 指纹图像数据
let imageBuffer = null
// 指纹设备宽高
let deviceWidth = 0
let deviceHeight = 0
// 算法句柄
let algorithmHandler = null
// 注册时，采集指纹数据数组
let registerTemplates: any[] = []

export function initFingerSDK() {
  isFingerLibExists()
  initFingerSensorSDK()
  initFingerAlgorithmSDK()
}

export function destroyFingerSDK() {
  destroyFingerSensorSDK()
  destroyFingerAlgorithmSDK()
}

/**
 * @description: 查询当前设备在线情况
 */
export function isFingerDeviceConnected(): boolean {
  const deviceCount = enumerateFingerSensors(deviceList, MAX_DEVICE_NUM)
  connected = deviceCount > 0
  return connected
}

/**
 * @description: 打开指纹仪设备
 */
export function openFingerDevice() {
  if (!connected) return false
  if (isOpen) return true

  // 开启设备
  deviceHandle = openFingerSensor(deviceList[0].ref())

  // 获取设备参数
  initDeviceParameters()
  // 初始化算法
  algorithmHandler = initFingerAlgorithm(deviceWidth, deviceHeight)

  const isDeviceOpened = deviceHandle.deref() !== null && algorithmHandler.deref() !== null
  isOpen = isDeviceOpened

  return isDeviceOpened
}

/**
 * @description: 关闭指纹仪设备
 */
export function closeFingerDevice() {
  if (!connected || !isOpen) return false

  // 关闭设备
  const deviceCloseResult = closeFingerSensor(deviceHandle)
  // 关闭算法
  const algorithmCloseResult = closeFingerAlgorithm(algorithmHandler)

  const isDeviceClosed = deviceCloseResult === 0 && algorithmCloseResult === 1
  // 重置指纹仪相关变量
  if (isDeviceClosed) {
    isOpen = false
    deviceHandle = null
    imageBuffer = null
    deviceWidth = 0
    deviceHeight = 0
    algorithmHandler = null
    registerTemplates = []
  }

  return isDeviceClosed
}

/**
 * @description: 获取指纹仪宽高
 */
function initDeviceParameters() {
  deviceWidth = getFingerSensorParameter(deviceHandle, 1)
  deviceHeight = getFingerSensorParameter(deviceHandle, 2)
  imageBuffer = new UcharType(deviceWidth * deviceHeight)
}

/**
 * @description: 开始采集指纹
 */
function captureFingerTemplate() {
  if (!connected || !isOpen) return false

  // 获取指纹仪捕获到的图像
  const captureResult = captureFingerprint(deviceHandle, imageBuffer, deviceWidth * deviceHeight)
  if (captureResult <= 0) return false

  const fingerTemplate = new UcharType(2048)
  // 提取图像
  const templateLength = extractFingerTemplate(
    algorithmHandler, 
    imageBuffer, 
    deviceWidth, 
    deviceHeight, 
    fingerTemplate, 
    2048
  )

  if (templateLength <= 0) return false

  return fingerTemplate
}

/**
 * @description: 注册指纹
 */
export function registerFinger(registerIndex: number) {
  const fingerTemplate = captureFingerTemplate()
  if (!fingerTemplate) return genResponseData(false, '采集指纹失败')

  if (registerIndex > 0) {
    // 对比前后两次采集的指纹
    const isMatched = verifyFingerTemplates(
      algorithmHandler, 
      registerTemplates[registerIndex - 1], 
      fingerTemplate
    )
    if (!isMatched) {
      registerTemplates = []
      return genResponseData(false, '请按压同一个手指')
    }
  }

  registerTemplates[registerIndex] = fingerTemplate
  registerIndex++

  if (registerIndex !== MAX_REGISTRATION_COUNT)
    return genResponseData(true, `还需要按压${MAX_REGISTRATION_COUNT - registerIndex}次手指`)

  const regTemplates = new TemplateType(registerTemplates)
  const finalTemplate = new UcharType(TEMPLATE_BYTE_LENGTH)
  const { success: genSuccess, result: genResult } = generateFingerTemplate(
    algorithmHandler,
    regTemplates,
    MAX_REGISTRATION_COUNT,
    finalTemplate,
  )

  if (!genSuccess) {
    registerTemplates = []
    return genResponseData(false, `生成登记模板失败，错误代码 = ${genResult}`)
  }

  const { success: addSuccess, result: addResult } = addFingerTemplateToDb(
    algorithmHandler, 
    9999, 
    genResult, 
    finalTemplate
  )

  registerTemplates = []
  
  if (!addSuccess) {
    return genResponseData(false, `添加指纹失败，错误代码 = ${addResult}`)
  }

  return genResponseData(true, '指纹注册成功', {
    templateData: finalTemplate.buffer.toString('base64')
  })
}

/**
 * @description: 识别指纹
 */
export function identifyFinger() {
  const fingerTemplate = captureFingerTemplate()
  if (!fingerTemplate) return genResponseData(false, '采集指纹失败')

  const matchScore = new IntType(1)
  const matchedId = new IntType(1)
  const identifyResult = identifyFingerTemplate(
    algorithmHandler, 
    fingerTemplate, 
    matchedId, 
    matchScore
  )
  
  return genResponseData(
    identifyResult === 1,
    identifyResult === 1 ? '识别成功' : '识别失败',
    { matchedId: matchedId[0] }
  )
}
