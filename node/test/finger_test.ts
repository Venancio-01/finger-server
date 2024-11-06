import { FingerService } from '../src/finger_service'
import process from 'node:process'

async function runTest() {
    const service = new FingerService()
    
    try {
        // 测试初始化
        console.log('Testing SDK initialization...')
        const initResult = await service.initSDK()
        console.log('Init result:', initResult)
        
        // 测试设备连接
        console.log('\nTesting device connection...')
        const connResult = await service.isDeviceConnected()
        console.log('Connection result:', connResult)
        
    } catch (error) {
        console.error('Test failed:', error)
    } finally {
        // 确保进程被清理
        process.exit(0)
    }
}

runTest() 
