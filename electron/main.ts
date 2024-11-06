import { app } from 'electron';
import { FingerService } from './finger_service';

let fingerService: FingerService | null = null;

app.whenReady().then(async () => {
  try {
    // 应用启动时初始化
    fingerService = FingerService.getInstance();
    await fingerService.initialize();
    
    // 创建窗口等其他操作...
    
  } catch (error) {
    console.error('Failed to initialize finger service:', error);
    app.quit();
  }
});

// 在应用退出前清理资源
app.on('before-quit', async (event) => {
  if (fingerService) {
    event.preventDefault(); // 阻止立即退出
    try {
      await fingerService.cleanup();
      fingerService = null;
      app.quit(); // 清理完成后退出
    } catch (error) {
      console.error('Failed to cleanup finger service:', error);
      app.quit(); // 即使清理失败也要退出
    }
  }
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
}); 
