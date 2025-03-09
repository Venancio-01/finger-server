# 指纹识别服务器 (Finger Server)

基于 ZKFinger SDK 开发的指纹识别 HTTP 服务器，提供指纹采集、注册、验证和识别等功能。

## 功能特点

- 基于 HTTP 协议，提供 RESTful API 接口
- 支持指纹设备的连接管理
- 支持指纹图像采集
- 支持指纹特征提取
- 支持指纹 1:1 比对
- 支持指纹 1:N 识别
- 支持指纹模板注册和管理
- 支持多种指纹采集器设备

## 系统要求

- 操作系统：Linux (支持 Ubuntu 12.04+、Deepin OS、UOS、银河麒麟等)
- CPU 架构：支持 X86_64、ARM64、MIPS64 等
- 依赖库：
  - libzkfpcap.so - 采集器接口动态库
  - libzkfp.so - 算法接口动态库
  - libzkalg12.so - ZKFinger10.0 算法核心库
  - 其他系统依赖库

## 安装说明

1. 确保系统已安装必需的动态库：
```bash
sudo cp lib/*.so /usr/local/lib/
sudo ldconfig
```

2. 编译项目：
```bash
mkdir build && cd build
cmake ..
make
```

3. 运行服务：
```bash
sudo ./start.sh
```

## API 接口说明

服务默认监听 `22813` 端口，所有接口采用 POST 方法，请求和响应数据格式均为 JSON。

### 1. 设备连接状态检查

- 请求：
```json
{
    "cmd": "isConnected"
}
```
- 响应：
```json
{
    "success": true
}
```

### 2. 打开设备

- 请求：
```json
{
    "cmd": "openDevice"
}
```
- 响应：
```json
{
    "success": true
}
```

### 3. 关闭设备

- 请求：
```json
{
    "cmd": "closeDevice"
}
```
- 响应：
```json
{
    "success": true
}
```

### 4. 采集指纹

- 请求：
```json
{
    "cmd": "capture"
}
```
- 响应：
```json
{
    "success": true,
    "template": "base64编码的指纹模板数据",
    "length": 1024
}
```

### 5. 加载指纹模板

- 请求：
```json
{
    "cmd": "loadTemplates",
    "templates": [
        {
            "id": 1,
            "template": "base64编码的指纹模板数据"
        }
    ]
}
```
- 响应：
```json
{
    "success": true
}
```

### 6. 指纹验证 (1:1)

- 请求：
```json
{
    "cmd": "verify",
    "templateData1": "base64编码的指纹模板1",
    "templateData2": "base64编码的指纹模板2"
}
```
- 响应：
```json
{
    "success": true,
    "score": 80
}
```

### 7. 指纹识别 (1:N)

- 请求：
```json
{
    "cmd": "identify",
    "template": "base64编码的指纹模板"
}
```
- 响应：
```json
{
    "success": true,
    "matchedId": 1,
    "score": 85,
    "threshold": 70
}
```

### 8. 注册指纹

- 请求：
```json
{
    "cmd": "register",
    "templateData": [
        "base64编码的指纹模板1",
        "base64编码的指纹模板2",
        "base64编码的指纹模板3"
    ]
}
```
- 响应：
```json
{
    "success": true,
    "templateData": "base64编码的最终注册模板"
}
```

## 错误处理

所有接口在发生错误时会返回：
```json
{
    "success": false,
    "error": "错误描述信息"
}
```

## 性能参数

- 指纹模板大小：≤1664 字节
- 指纹旋转支持：0~360 度
- FAR（误识率）：≤0.001%
- FRR（拒识率）：≤1%
- 1:1 比对速度：≤50ms
- 1:N 识别速度：≤500ms (N=50,000)
- 图像要求：500 DPI

## 注意事项

1. 使用前请确保指纹设备已正确连接
2. 建议每次采集完成后关闭设备
3. 指纹模板数据使用 Base64 编码传输
4. 1:1 比对推荐阈值为 50 分
5. 1:N 识别推荐阈值为 70 分
6. 注册指纹时建议采集 3 次以上样本 
