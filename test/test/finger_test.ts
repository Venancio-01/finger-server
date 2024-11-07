import axios from 'axios'
import process from 'node:process'
import { sleep } from '../src/utils'

const API_URL = 'http://localhost:22813'

async function sendCommand(cmd: string, data: any = {}) {
  try {
    const response = await axios.post(API_URL, {
      cmd,
      ...data
    })
    return response.data
  }
  catch (error: any) {
    console.error(`Command ${cmd} failed:`, error.response?.data || error.message)
    throw error
  }
}

async function runTest() {
  try {
    // 测试初始化
    console.log('\n测试 SDK 初始化...')
    const initResult = await sendCommand('init')
    console.log('初始化结果:', initResult)

    await sleep(1000)

    // 测试设备连接
    console.log('\n测试设备连接状态...')
    const connResult = await sendCommand('isConnected')
    console.log('连接状态:', connResult)

    await sleep(1000)

    // 测试打开设备
    console.log('\n测试打开设备...')
    const openResult = await sendCommand('openDevice')
    console.log('打开设备结果:', openResult)

    await sleep(1000)

    // // 测试注册指纹
    // console.log('\n测试指纹注册...')
    // console.log('请按压手指...')
    // for (let i = 0; i < 3; i++) {
    //   const registerResult = await sendCommand('register', { index: i })
    //   console.log(`第 ${i + 1} 次采集结果:`, registerResult)
    //   await sleep(3000)
    // }

    // await sleep(1000)

    // // 测试指纹识别
    // console.log('\n测试指纹识别...')
    // console.log('请按压手指...')
    // const identifyResult = await sendCommand('identify')
    // console.log('识别结果:', identifyResult)

    // await sleep(1000)

    // 测试关闭设备
    console.log('\n测试关闭设备...')
    const closeResult = await sendCommand('closeDevice')
    console.log('关闭设备结果:', closeResult)

    await sleep(1000)

    // 测试销毁 SDK
    console.log('\n测试销毁 SDK...')
    const destroyResult = await sendCommand('destroy')
    console.log('销毁结果:', destroyResult)

  }
  catch (error) {
    console.error('测试失败:', error)
  }
  finally {
    // 确保进程退出
    process.exit(0)
  }
}

// 添加错误处理
process.on('unhandledRejection', (error) => {
  console.error('未处理的 Promise 拒绝:', error)
  process.exit(1)
})

runTest()
