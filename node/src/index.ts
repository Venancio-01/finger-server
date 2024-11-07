#!/usr/bin/env node
import { createInterface } from 'readline'
import { genResponseData } from './utils'
import {
  closeFingerDevice,
  destroyFingerSDK,
  identifyFinger,
  initFingerSDK,
  isFingerDeviceConnected,
  openFingerDevice,
  registerFinger,
} from './main'

// 创建命令行接口
const rl = createInterface({
  input: process.stdin,
  output: process.stdout,
  terminal: false
})

// 处理命令
async function processCommand(command: any) {
  try {
    switch (command.cmd) {
      case 'init':
        initFingerSDK()
        return genResponseData(true, 'SDK 初始化成功')
      
      case 'isConnected':
        const connected = isFingerDeviceConnected()
        return genResponseData(true, undefined, { connected })
      
      case 'openDevice':
        const success = openFingerDevice()
        return genResponseData(success, success ? '设备打开成功' : '设备打开失败')
      
      case 'register':
        return registerFinger(command.index)
      
      case 'identify':
        return identifyFinger()
      
      case 'closeDevice':
        const closeSuccess = closeFingerDevice()
        return genResponseData(closeSuccess, closeSuccess ? '设备关闭成功' : '设备关闭失败')
      
      case 'destroy':
        destroyFingerSDK()
        return genResponseData(true, 'SDK 销毁成功')
      
      default:
        return genResponseData(false, '未知命令')
    }
  }
  catch (error: any) {
    return genResponseData(false, `命令执行失败: ${error.message}`)
  }
}

console.log('指纹识别服务已启动，等待命令输入...')

// 持续监听输入
rl.on('line', async (line) => {
  try {
    const command = JSON.parse(line)
    const result = await processCommand(command)
    console.log(JSON.stringify(result))
  }
  catch (error: any) {
    console.error(JSON.stringify(genResponseData(false, `命令解析失败: ${error.message}`)))
  }
})

// 处理退出信号
process.on('SIGINT', () => {
  console.log('正在关闭服务...')
  destroyFingerSDK()
  rl.close()
  process.exit(0)
})

process.on('SIGTERM', () => {
  console.log('正在关闭服务...')
  destroyFingerSDK()
  rl.close()
  process.exit(0)
})
