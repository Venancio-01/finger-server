import { spawn } from 'child_process'

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
  async initialize() {
    if (this.initialized) return;
    
    // 初始化 SDK
    const initResult = await this.sendCommand({ cmd: 'initialize' });
    if (!initResult.success) {
      throw new Error('Failed to initialize SDK');
    }
    
    // 检查设备连接
    const connResult = await this.sendCommand({ cmd: 'isConnected' });
    if (!connResult.connected) {
      throw new Error('No device connected');
    }
    
    // 打开设备
    const openResult = await this.sendCommand({ cmd: 'openDevice' });
    if (!openResult.success) {
      throw new Error('Failed to open device');
    }
    
    this.initialized = true;
  }

  // 应用关闭时调用
  async cleanup() {
    if (!this.initialized) return;
    
    try {
      // 关闭设备
      await this.sendCommand({ cmd: 'closeDevice' });
      // 销毁 SDK
      await this.sendCommand({ cmd: 'uninitialize' });
      this.initialized = false;
    } finally {
      if (this.process) {
        this.process.kill();
        this.process = null;
      }
    }
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
