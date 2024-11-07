import { spawn } from 'child_process'

interface Command {
  cmd: string
}

interface InitResult {
  success: boolean
  deviceInit?: boolean
  algorithmInit?: boolean
}

interface DestroyResult {
  success: boolean
}


interface OpenDeviceResult {
  success: boolean
  error?: string
}

export class FingerService {
  private static instance: FingerService | null = null;
  private process: any;
  private initialized = false;
  
  private constructor() {
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

  // 单例模式
  public static getInstance(): FingerService {
    if (!FingerService.instance) {
      FingerService.instance = new FingerService();
    }
    return FingerService.instance;
  }

  // 应用启动时调用
  async initialize(): Promise<InitResult> {
    if (this.initialized) {
      return {
        success: true,
      }
    } ;
    
    // 初始化 SDK
    const initResult = await this.sendCommand<InitResult>({ cmd: 'initialize' });
    if (!initResult.success) {
      throw new Error('Failed to initialize SDK');
    }
    
    this.initialized = true;

    return initResult
  }

  // 应用关闭时调用
  async cleanup(): Promise<DestroyResult> {
    if (!this.initialized) {
      return {
        success: true,
      }
    } ;
    
    try {
      // 销毁 SDK
      await this.sendCommand({ cmd: 'uninitialize' });
      this.initialized = false;
      return {
        success: true,
      }
    } finally {
      if (this.process) {
        this.process.kill();
        this.process = null;
      }
    }
  }

  // 打开设备
  async openDevice(): Promise<OpenDeviceResult> {
    return this.sendCommand<OpenDeviceResult>({ cmd: 'openDevice' })
  }

  // 页面相关的操作只需要调用具体的指纹功能
  async register(/* params */) {
    if (!this.initialized) {
      throw new Error('Service not initialized');
    }
    // 执行注册逻辑
  }

  async identify(/* params */) {
    if (!this.initialized) {
      throw new Error('Service not initialized');
    }
    // 执行识别逻辑
  }

  private async sendCommand<T>(command: Command): Promise<T> {
    return new Promise((resolve, reject) => {
      const cmd = JSON.stringify(command)
      this.process.stdin.write(cmd + '\n')
      
      const handler = (data: Buffer) => {
        try {
          const str = data.toString().trim()
          // 尝试解析为 JSON，如果失败则返回原始字符串
          const response = str.startsWith('{') || str.startsWith('[') ? 
            JSON.parse(str) : 
            { success: false, message: str }
          resolve(response)
        } catch (e: any) {
          // JSON 解析失败时,返回错误信息
          reject(new Error(`解析响应失败: ${e.message}`))
        }
      }

      this.process.stdout.once('data', handler)
    })
  }

  async isConnected() {
    return this.sendCommand({ cmd: 'isConnected' })
  }

  private handleResponse(data: Buffer) {
    console.log('Response:', data.toString())
  }

  private handleError(data: Buffer) {
    console.error('Error:', data.toString())
  }
} 
