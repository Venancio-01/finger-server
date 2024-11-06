import { FingerService } from "../src/finger_service";
import process from "node:process";

async function runTest() {
  const service = FingerService.getInstance();

  try {
    // 测试初始化
    console.log("Testing SDK initialization...");
    const initResult = await service.initialize();
    console.log("Init result:", initResult);

    // 测试设备连接
    console.log("\nTesting device connection...");
    const connResult = await service.isConnected();
    console.log("Connection result:", connResult);

    // 测试打开设备
    console.log("\nTesting device opening...");
    const openResult = await service.openDevice();
    console.log("Open result:", openResult);

    setTimeout(async () => {
      // 测试销毁
      console.log("\nTesting SDK destruction...");
      const destroyResult = await service.cleanup();
      console.log("Destroy result:", destroyResult);
    }, 2000);
  } catch (error) {
    console.error("Test failed:", error);
  } finally {
    // 确保进程被清理
    process.exit(0);
  }
}

runTest();
