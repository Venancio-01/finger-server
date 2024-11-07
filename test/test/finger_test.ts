import axios from "axios";
import process from "node:process";
import { sleep } from "../src/utils";

const API_URL = "http://localhost:22813";

async function sendCommand(cmd: string, data: any = {}) {
  try {
    const response = await axios.post(API_URL, {
      cmd,
      ...data,
    });
    return response.data;
  } catch (error: any) {
    console.error(`Command ${cmd} failed:`, error.response?.data || error.message);
    throw error;
  }
}

async function runTest() {
  try {
    // 测试设备连接
    console.log("\n测试设备连接状态...");
    const connResult = await sendCommand("isConnected");
    console.log("连接状态:", connResult);

    await sleep(100);

    // 测试打开设备
    console.log("\n测试打开设备...");
    const openResult = await sendCommand("openDevice");
    console.log("打开设备结果:", openResult);

    await sleep(1000);

    // 测试指纹识别
    console.log("\n测试指纹识别...");
    console.log("请按压手指...");
    while (true) {
      const identifyResult = await sendCommand("capture");
      const { success, data, error } = identifyResult;
      if (success) {
        console.log("识别结果:", data);
        break;
      } else {
        console.log("识别失败:", error);
      }
      await sleep(200);
    }

    // 测试关闭设备
    console.log("\n测试关闭设备...");
    const closeResult = await sendCommand("closeDevice");
    console.log("关闭设备结果:", closeResult);
  } catch (error) {
    console.error("测试失败:", error);
  } finally {
    process.exit(0);
  }
}

// 添加错误处理
process.on("unhandledRejection", error => {
  console.error("未处理的 Promise 拒绝:", error);
  process.exit(1);
});

runTest();
