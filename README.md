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

## 编译环境要求

- GCC/G++ 7.0 及以上版本（支持 C++17）
- CMake 3.10 及以上版本
- vcpkg 包管理器
- 系统依赖：
  - cpprestsdk
  - nlohmann_json
  - pthread
  - dl (动态链接库)

## 安装说明

### 1. 安装编译依赖

```bash
# 安装基本编译工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install

# 安装项目依赖
./vcpkg install cpprestsdk:x64-linux
./vcpkg install nlohmann-json:x64-linux
```

### 2. 安装 SDK 动态库

```bash
# 复制 SDK 动态库到系统目录
sudo cp lib/*.so /usr/local/lib/
sudo ldconfig
```

### 3. 编译项目

```bash
# 创建并进入构建目录
mkdir build && cd build

# 配置 CMake 项目
# 注意：请将 ~/vcpkg 替换为你的 vcpkg 实际安装路径
cmake -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake ..

# 编译
make -j$(nproc)
```

### 4. 运行服务

```bash
sudo ./start.sh
```

## 开发说明

### 项目结构
```
finger-server/
├── CMakeLists.txt          # CMake 构建配置文件
├── main.cpp                # 主程序入口
├── finger_device.h         # 设备操作相关头文件
├── finger_device.cpp       # 设备操作实现
├── finger_algorithm.h      # 算法相关头文件
├── finger_algorithm.cpp    # 算法实现
├── base64.h               # Base64 编解码头文件
├── base64.cpp             # Base64 编解码实现
├── lib/                   # SDK 动态库目录
│   ├── libzkfpcap.so
│   ├── libzkfp.so
│   └── libzkalg12.so
├── doc/                   # 文档目录
│   └── ZKFinger SDK.md   # SDK 开发文档
└── start.sh              # 服务启动脚本
```

### 编译选项说明

CMakeLists.txt 中的主要配置：

- CMAKE_CXX_STANDARD：设置为 17，启用 C++17 特性
- CMAKE_TOOLCHAIN_FILE：指定 vcpkg 工具链文件路径
- 依赖项：
  - cpprestsdk：用于 HTTP 服务器功能
  - nlohmann_json：用于 JSON 数据处理
  - pthread：用于多线程支持
  - dl：用于动态加载库

### 常见编译问题

1. CMake 找不到 vcpkg 工具链
   ```bash
   # 解决方法：指定 vcpkg 根目录
   export VCPKG_ROOT=~/vcpkg
   cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..
   ```

2. 找不到 SDK 动态库
   ```bash
   # 解决方法：确保动态库已正确安装
   sudo ldconfig -v | grep zkfp
   ```

3. 编译器版本过低
   ```bash
   # 解决方法：安装更新的 GCC/G++
   sudo apt-get install -y gcc-9 g++-9
   export CC=gcc-9
   export CXX=g++-9
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
