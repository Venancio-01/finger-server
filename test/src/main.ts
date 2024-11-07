import { Buffer } from 'buffer'
import { genResponseData } from './utils'
import { MAX_DEVICE_NUM, MAX_REGISTRATION_COUNT, TEMPLATE_BYTE_LENGTH } from './utils/config'
import type { RfidFingerUser } from './database'
import { insertRfidFingerUser, rfid_finger_user, selectRfidFingerUser, selectRfidFingerUserList, updateRfidFingerUser } from './database'
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
let registerTemplates = []

// 指纹对应的用户数据
let fingerUserList: RfidFingerUser[] = []

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
 * @return {*}
 */
export function isFingerDeviceConnected(): boolean {
  const deviceCount = enumerateFingerSensors(deviceList, MAX_DEVICE_NUM)
  info(`指纹仪连接数量：${deviceCount}`)
  connected = deviceCount > 0
  return connected
}

/**
 * @description: 打开指纹仪设备
 * @return {*}
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

  loadFingerTemplates()

  return isDeviceOpened
}

/**
 * @description: 关闭指纹仪设备
 * @return {*}
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
 * @return {*}
 */
export function initDeviceParameters() {
  deviceWidth = getFingerSensorParameter(deviceHandle, 1)
  deviceHeight = getFingerSensorParameter(deviceHandle, 2)
  imageBuffer = new UcharType(deviceWidth * deviceHeight)
}

/**
 * @description: 开始采集指纹
 * @return {*}
 */
export function captureFingerTemplate() {
  if (!connected || !isOpen) return false

  // 获取指纹仪捕获到的图像
  const captureResult = captureFingerprint(deviceHandle, imageBuffer, deviceWidth * deviceHeight)
  if (captureResult <= 0) return false

  const fingerTemplate = new UcharType(2048)
  // 提取图像
  const templateLength = extractFingerTemplate(algorithmHandler, imageBuffer, deviceWidth, deviceHeight, fingerTemplate, 2048)

  if (templateLength <= 0) return false

  return fingerTemplate
}

export async function handleFingerRegister(userId: number, order: FingerOrder, registerCurrentIndex: number) {
  let registerResult = null
  const fingerTemplate = captureFingerTemplate()

  if (fingerTemplate) registerResult = await registerFinger(fingerTemplate, userId, order, registerCurrentIndex)

  return registerResult
}

/**
 * @description: 注册指纹
 * @return {*}
 */
export async function registerFinger(fingerTemplate, userId: number, order: FingerOrder, registerCurrentIndex: number) {
  const resetRegistration = () => {
    registerTemplates = []
  }

  const { success: isRegistered } = identifyFinger(fingerTemplate)
  if (isRegistered) {
    resetRegistration()
    return genResponseData(false, '登记失败，当前手指已登记', { alert: true })
  }

  if (registerCurrentIndex >= MAX_REGISTRATION_COUNT) {
    resetRegistration()
    return genResponseData(false)
  }

  if (registerCurrentIndex > 0) {
    // 对比前后两次采集的指纹
    const isMatched = verifyFingerTemplates(algorithmHandler, registerTemplates[registerCurrentIndex - 1], fingerTemplate)
    if (!isMatched) {
      resetRegistration()
      return genResponseData(false, '登记失败，请按压同一个手指', {
        alert: true,
      })
    }
  }

  registerTemplates[registerCurrentIndex] = fingerTemplate
  registerCurrentIndex++

  if (registerCurrentIndex !== MAX_REGISTRATION_COUNT)
    return genResponseData(true, `您还需要按压${MAX_REGISTRATION_COUNT - registerCurrentIndex}次手指`)

  const regTemplates = new TemplateType(registerTemplates)
  const finalTemplate = new UcharType(TEMPLATE_BYTE_LENGTH)
  const { success: genSuccess, result: genResult } = generateFingerTemplate(
    algorithmHandler,
    regTemplates,
    MAX_REGISTRATION_COUNT,
    finalTemplate,
  )

  if (!genSuccess) {
    resetRegistration()
    return genResponseData(false, `生成登记模板失败，错误代码 = ${genResult}`, { alert: true })
  }
  const { success: addSuccess, result: addResult } = addFingerTemplateToDb(algorithmHandler, 9999, genResult, finalTemplate)

  if (!addSuccess) {
    resetRegistration()
    return genResponseData(true, `添加指纹失败，错误代码 = ${addResult}`, {
      alert: true,
    })
  }

  const userCondition = and(
    eq(rfid_finger_user.Userid, Number(userId)),
    eq(rfid_finger_user.order, order),
  )
  const existingFingerData = await selectRfidFingerUser(userCondition)

  const templateData = finalTemplate.buffer.toString('base64')
  const orderText = order === 1 ? '一' : '二'
  if (existingFingerData !== null) {
    try {
      await updateRfidFingerUser(
        userCondition,
        {
          FingerData: templateData,
        },
      )

      resetRegistration()
      return genResponseData(true, `指纹${orderText}更新成功`, {
        registerSuccess: true,
        alert: true,
      })
    }
    catch (e) {
      resetRegistration()
      return genResponseData(false, `指纹${orderText}更新失败`, {
        alert: true,
      })
    }
  }
  else {
    try {
      await insertRfidFingerUser({
        FingerData: templateData,
        order,
        Userid: Number(userId),
        CreateDate: generateCurrentTime(),
      })

      resetRegistration()
      return genResponseData(true, `指纹${orderText}添加成功`, {
        registerSuccess: true,
        alert: true,
      })
    }
    catch (e) {
      resetRegistration()
      return genResponseData(false, `指纹${orderText}添加失败`, {
        alert: true,
      })
    }
  }
}

/**
 * @description: 识别指纹
 * @return {*}
 */
export function identifyFinger(fingerTemplate) {
  const matchScore = new IntType(1)
  const matchedId = new IntType(1)
  const identifyResult = identifyFingerTemplate(algorithmHandler, fingerTemplate, matchedId, matchScore)
  const isIdentified = identifyResult === 1
  const message = isIdentified ? '识别成功!' : '识别失败'
  const userIndex = matchedId[0] - 1
  const userId = fingerUserList[userIndex]?.Userid
  return genResponseData(isIdentified, message, userId)
}

export function handleFingerIdentify() {
  let identifyResult = null
  const fingerTemplate = captureFingerTemplate()

  if (fingerTemplate) identifyResult = identifyFinger(fingerTemplate)

  return identifyResult
}

/**
 * @description: 加载数据库指纹模板到内存
 * @return {*}
 */
export async function loadFingerTemplates() {
  fingerUserList = await selectRfidFingerUserList()
  if (fingerUserList.length === 0) return

  fingerUserList.forEach((user) => {
    if (user.FingerData) {
      const templateBuffer = Buffer.from(user.FingerData, 'base64')
      addFingerTemplateToDb(algorithmHandler, user.Userid, TEMPLATE_BYTE_LENGTH, templateBuffer)
    }
  })
}
