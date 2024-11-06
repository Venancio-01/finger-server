import { spawn } from 'child_process'

export class FingerService {
  private process: any
  
  constructor() {
    // 获取当前文件所在目录
    const currentDir = __dirname
    // 构建可执行文件的完整路径
    const exePath = require('path').join(currentDir, '../../build/finger_cli')
    
    this.process = spawn(exePath, [], {
      stdio: ['pipe', 'pipe', 'pipe']
    })

    this.process.stdout.on('data', this.handleResponse.bind(this))
    this.process.stderr.on('data', this.handleError.bind(this))
  }

  private async sendCommand(command: any): Promise<any> {
    return new Promise((resolve, reject) => {
      const cmd = JSON.stringify(command)
      this.process.stdin.write(cmd + '\n')
      
      const handler = (data: Buffer) => {
        try {
          const response = JSON.parse(data.toString())
          resolve(response)
        } catch (e) {
          reject(e)
        }
      }

      this.process.stdout.once('data', handler)
    })
  }

  async initSDK() {
    return this.sendCommand({ cmd: 'init' })
  }

  async isDeviceConnected() {
    return this.sendCommand({ cmd: 'isConnected' })
  }

  private handleResponse(data: Buffer) {
    console.log('Response:', data.toString())
  }

  private handleError(data: Buffer) {
    console.error('Error:', data.toString())
  }
} 
