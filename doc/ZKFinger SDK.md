ZKFinger SDK For Linux
接口开发⼿册
熵基科技股份有限公司
<http://www.zkteco.com>
版权声明
熵基科技股份有限公司版权所有©,保留⼀切权利。
⾮经本公司书⾯许可,任何单位和个⼈不得擅⾃摘抄、复制本书的部分或全部,并不得以任何形式传播。

1. ⽬录
版权声明
1. ⽬录
2. 前⾔
3. ZKFinger10.0 介绍
3. 技术规格
3.1. 开发语⾔
3.2. 平台⽀持
3.3. SDK 架构
3.4. 技术参数
3.5. 快速集成
3.5.1. SDK⽂件列表
3.5.2. ⽀持设备列表
3.6. 编程引导
3.6.1. 登记流程
3.6.2. ⽐对流程
4. SDK接⼝说明
4.1. 设备接⼝说明
4.1.1. 功能列表
4.1.2. sensorEnumDevices
4.1.3. sensorOpen
4.1.4. sensorClose
4.1.5. sensorRebootEx
4.1.6. sensorCapture
4.1.7. sensorGetParameter
4.1.8. sensorSetParameter
4.1.9. sensorGetParameterEx
4.1.10. sensorSetParameterEx
4.1.11. sensorReboot
4.1.12. sensorStatus
4.2. 算法接⼝说明
4.2.1. 功能列表
4.2.2. BIOKEY_INIT_SIMPLE
4.2.3. BIOKEY_CLOSE
4.2.4. BIOKEY_EXTRACT
4.2.5. BIOKEY_GETLASTQUALITY
4.2.6. BIOKEY_GENTEMPLATE
4.2.7. BIOKEY_VERIFY
4.2.8. BIOKEY_SET_PARAMETER
4.2.9. BIOKEY_DB_ADD
4.2.10. BIOKEY_DB_DEL
4.2.11. BIOKEY_DB_CLEAR
4.2.12. BIOKEY_DB_COUNT
4.2.13. BIOKEY_IDENTIFYTEMP
4.2.14. BIOKEY_EXTRACT_GRAYSCALEDATA
5. 附录
5.1 名词解释
CPU平台 系统版本
X86_64（Intel/AMD/海
光/兆芯等）
Ubuntu12.04+
Deepin OS V15.11+
UOS 20
银河麒麟V4/V10
ARM64(⻜腾、鲲鹏等)
UOS 20
麒麟系统V4/V10
MIPS64(⻰芯)
UOS 20
麒麟系统V4//V10
其他Linux 系统/CPU等
根据不同系统、不同CPU定制评估适配(⼤部分客⼾可以拿以上版本⾃⾏
验证；有不满⾜再⾛定制流程)
2. 前⾔
本⽂档会为您提供 SDK 基本的开发指南和技术背景介绍,帮助您更好地使⽤我们的技术服务。
衷⼼感谢您对我们技术与产品的信任和⽀持!
3. ZKFinger10.0 介绍
熵基科技⼀直专注于指纹识别算法的研究和产业化推⼴，已将指纹识别系统应⽤到各个⾏业中。随着指
纹识别系统越来越⼴泛的应⽤，市场对指纹识别算法的精确性,适⽤性和运算速度等多⽅⾯提出更⾼的要
求。为满⾜这些需求，我们从低质量指纹图像的增强,指纹的特征提取、指纹图像的分类与检索及压缩技
术、指纹图像匹配算法等多⽅⾯进⾏优化，推出ZKFinger10.0版⾼速算法。该算法在⼤规模的数据库上
进⾏了严格的测试，误识率（False Accept Rate, FAR）、拒识率（False Reject Rate, FRR）、拒登率
（Error Registration Rate, ERR）等性能都⼤⼤提⾼，对过⼲、太湿、伤疤、脱⽪等低质量的指纹图像
处理效果也明显增强，算法⽐对速度提升了10倍以上。该算法的指纹模板也同时进⾏了优化存储，与以
前的算法版本的指纹模板不兼容。您选择ZKFinger10.0版⾼速算法后，必须重新登记⽤⼾指纹模板。
ZKFinger10.0 算法具有以下特点：
ZKFinger SDK软件开发包能够快速集成到客⼾系统中，通过开放图像处理接⼝，可以⽀持任何扫
描设备和指纹Senor(图像DPI>=300DPI)。
ZKFinger10.0算法通过⾃适应的、适合匹配的滤镜和恰当的阈值，减弱图⽚噪声，增强脊和⾕的
对⽐度，甚⾄能够从质量很差的指纹（脏、⼑伤、疤、痕、⼲燥、湿润或撕破）中获取前档的全局
和局部特征点。
ZKFinger10.0算法⽀持180°旋转⽐对。
ZKFinger10.0算法不需要指纹必须有全局特征点（核⼼点、三⻆点等），通过局部特征点就可以
完成识别。
ZKFinger10.0算法通过分类算法（指纹被分成五⼤类型：拱形、左环类、右环类、尖拱类、漩涡
类“⽃”），预先使⽤全局特征排序，从⽽⼤⼤的加速指纹匹配过程。
3. 技术规格
3.1. 开发语⾔
⽀持各种主流桌⾯开发语⾔(如C、C++、Java(jna)等)
3.2. 平台⽀持
3.3. SDK 架构
参数名 描述
模板⼤⼩ <=1664字节
旋转 0~360度
FAR <=0.001%
FRR <=1%
⽐对速度（1:1) <=50ms
识别速度（1:50,000) <=500ms
图像DPI 500DPI
⽂件名 描述 其他备注
libzksensorcore.so 采集器通讯核⼼库
libidfprcap.so 免驱指纹采集器采集动态库
libslkidcap.so Live10R/Live20R 采集器采集动态库
libzkfpcap.so 采集器接⼝动态库 对应⽂档设备API
libzkfp.so 算法接⼝动态库 对应⽂档算法API
libzkalg12.so ZKFinger10.0算法核⼼库
libusb-0.1.so.4 算法核⼼依赖 仅X86_64需要
libusb-1.0.so.0.1.0 算法核⼼依赖 仅X86_64需要
设备名 VID PID
Live10R 0x1B55 0x0124
Live20R 0x1B55 0x0120
3.4. 技术参数
3.5. 快速集成
3.5.1. SDK⽂件列表
3.5.2. ⽀持设备列表
设备名 VID PID
FS200(-R) 0x1B55 0x0304
FS300(-R) 0x1B55 0x0306
ZK6000A(-R) 0x1B55 0x0308
ZK7000A(-R) 0x1B55 0x0302
3.6. 编程引导
ZKFinger SDK 单独提供采集SDK和算法SDK,其中采集SDK只是3.5.2 设备列表列举的设备；算法SDK版
本为ZKFinger10.0，由于算法绑定了采集器，因此使⽤时需要在调⽤采集SDK连接设备成功后才可以初
始化算法。其中FS200/FS300/ZK6000A/ZK7000A 内置加密芯⽚需烧写ZKFinger10.0许可才可以使⽤
本算法。
以下将对登记⽐对流程进⾏简单介绍。
3.6.1. 登记流程
调⽤sensorCapture采集图像
当采集图像成功后调⽤提取模板；失败则继续采集
提取模板失败时继续采集图像；提取成功时调⽤identify判断是否指纹已登记
如果⼿指已登记结束登记过程并提⽰⽤⼾；采集3次指纹模板，不⾜三次继续采集图像
⽣成登记模板，当失败时结束登记过程并提⽰⽤⼾；成功时保存数据库
函数 功能描述
sensorEnumDevices 枚举设备，搜索全部本SDK⽀持的设备列表
sensorOpen 打开设备
sensorClose 关闭设备
sensorRebootEx 重启设备(打开->重启->关闭)
sensorCapture 采集图像
完成登记过程
3.6.2. ⽐对流程
1:N识别⾸先需要将已登记的模板都加载到内存中；可在算法初始化成功之后加载
调⽤采集SDK采集图像
当采集失败时继续采集；当采集成功时调⽤算法SDK提取模板
当提取模板失败时继续采集图像；当提取模板成功时调⽤identify⽐对指纹，并反馈结果给⽤⼾
完成⽐对过程
4. SDK接⼝说明
4.1. 设备接⼝说明
4.1.1. 功能列表
函数 功能描述
sensorGetParameter(Ex) 获取参数
sensorSetParameter(Ex) 设置参数
sensorReboot 重启设备(设备已打开状态)
sensorStatus 获取设备状态
4.1.2. sensorEnumDevices
4.1.3. sensorOpen
4.1.4. sensorClose
4.1.5. sensorRebootEx
[函数]
int sensorEnumDevices(TXUSBDevice deviceList[], int nMaxCount);
[功能]
枚举设备
[参数]
deviceList[out]
返回设备数组
maxCount[in]
数组⼤小
[返回值]
获取⽀持的指纹设备数
[函数]
void*sensorOpen(PXUSBDevice device);
[功能]
打开指纹仪
[参数]
device[in]
设备结构体指针
[返回值]
设备句柄
[函数]
int sensorClose(void* handle);
[功能]
关闭设备
[参数]
handle[in]
设备句柄(⻅sensorOpen)
[返回值]
0 表⽰成功
其他失败
4.1.6. sensorCapture
4.1.7. sensorGetParameter
4.1.8. sensorSetParameter
[函数]
int sensorRebootEx(int index);
[功能]
重启设备(不需要先调⽤sensorOpen)
[参数]
index[in]
设备索引(云桌⾯版固定传0)
[返回值]
0 表⽰成功
其他失败
[函数]
int sensorCapture(void *handle, unsigned char *imageBuffer, int imageBufferSize);
[功能]
采集指纹图像
[参数]
handle[in]
设备句柄(⻅sensorOpen)
imageBuffer[out]
返回raw图像数据(由调⽤者申请内存，不小于width*height字节)
imageBufferSize[in]
imageBuffer 内存⼤小
[返回值]

>0 表⽰取像成功并返回数据⻓度
其他失败
[函数]
int sensorGetParameter(void *handle, int paramCode);
[功能]
获取简单参数
[参数]
handle[in]
设备句柄(⻅sensorOpen)
parmaCode[in]
参数代码
1: 指纹图像宽
2: 指纹图像⾼
[返回值]
参数值
[函数]
int sensorSetParameter(void*handle, int paramCode, int paramValue);
[功能]
设置参数
[参数]
handle[in]
设备句柄(⻅sensorOpen)
parmaCode[in]
参数代码
5: 取像模式
4.1.9. sensorGetParameterEx
4.1.10. sensorSetParameterEx
paramValue[in]
参数值,当paramCode=5, paramValue=0(默认值) 表⽰探测模式，paramValue=1表⽰流模式
[返回值]
0 表⽰成功
其他失败
[函数]
int sensorGetParameterEx(void *handle, int paramCode, char *paramValue, int *paramLen);
[功能]
获取参数
[参数]
handle[in]
设备句柄(⻅sensorOpen)
parmaCode[in]
参数代码
1: 指纹图像宽(int)
2: 指纹图像⾼(int)
1103: 设备序列号(String)
paramValue[out]
返回参数值(由调⽤者分配内存)
paramLen[in/out]
in: paramValue内存⼤小
out: 实际返回参数值数据⼤小
[返回值]
0 表⽰成功
其他失败
[⽰例]
//获取指纹图像宽
int width = 0;
int retSize = 4;
int ret = sensorGetParameterEx(handle, 1, (unsigned char*)&width, &retSize);
//获取设备序列号
char szSerialNumber[64];
int retSize = 64;
int ret = sensorGetParameterEx(handle, 1103, (unsigned char* szSerialNumber, &retSize);
[函数]
int sensorSetParameterEx(void *handle, int paramCode, char *paramValue, int paramLen);
[功能]
获取参数
[参数]
handle[in]
设备句柄(⻅sensorOpen)
parmaCode[in]
参数代码
1: 指纹图像宽(int)
2: 指纹图像⾼(int)
1103: 设备序列号(String)
paramValue[in]
参数值
paramLen[in]
参数值数据⻓度
函数 功能描述
BIOKEY_INIT_SIMPLE 初始化算法
BIOKEY_CLOSE 释放算法
BIOKEY_EXTRACT
提取模板(传⼊图像宽⾼必须与BIOKEY_INIT_SIMPLE初始
化宽⾼⼀致)
BIOKEY_GETLASTQUALITY 获取最近⼀次提取模板的模板质量
BIOKEY_GENTEMPLATE ⽣成登记模板
BIOKEY_VERIFY 1:1⽐对
BIOKEY_SET_PARAMETER 设置算法参数
BIOKEY_DB_ADD 添加登记模板到1:N内存中
BIOKEY_DB_DEL 删除模板
BIOKEY_DB_CLEAR 清空模板
BIOKEY_DB_COUNT 获取已添加模板数
BIOKEY_IDENTIFYTEMP 1:N ⽐对
BIOKEY_EXTRACT_GRAYSCALEDATA 提取模板
4.1.11. sensorReboot
4.1.12. sensorStatus
4.2. 算法接⼝说明
4.2.1. 功能列表
4.2.2. BIOKEY_INIT_SIMPLE
[返回值]
0 表⽰成功
其他失败
[备注]
本接口⽬前不需要使⽤到
[函数]
int sensorReboot(void* handle)
[功能]
重启设备(需先调⽤sensorOpen成功)
[参数]
handle[in]
设备句柄(⻅sensorOpen)
[返回值]
0 表⽰成功
其他失败
[函数]
int sensorStatus(void* handle)
[功能]
获取设备状态
[参数]
handle[in]
设备句柄(⻅sensorOpen)
[返回值]
0 正常
-99998 SLK20R bulk端点异常，需要重启设备
其他错误 (连续多次(如5次)失败后，重启设备)
4.2.3. BIOKEY_CLOSE
4.2.4. BIOKEY_EXTRACT
4.2.5. BIOKEY_GETLASTQUALITY
[函数]
void* BIOKEY_INIT_SIMPLE(int License, int width, int height, BYTE *Buffer);
[功能]
初始化算法
[参数]
license[in]
固定传0
width[in]
指纹图像宽
height[in]
指纹图像⾼
Buffer[in]
固定传NULL
[返回值]
算法句柄
[备注]
设备[sensorOpen]连接成功后⽅可调⽤算法初始化
[函数]
int BIOKEY_CLOSE(HANDLE Handle);
[功能]
释放算法
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
[返回值]
1 表⽰成功
其余表⽰失败
[函数]
int BIOKEY_EXTRACT(HANDLE Handle, BYTE* PixelsBuffer, BYTE *Template, int PurposeMode);
[功能]
提取指纹模板
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
pixelsBuffer[in]
指纹RAW图像数据(宽⾼必须与BIOKEY_INIT_SIMPLE⼀致)，⻅sensorCapture
Template[out]
返回指纹模板数据(建议分配2048字节)
PurposeMode[in]
固定传0
[返回值]
> 0 表⽰提取成功，返回模板数据实际⻓度
4.2.6. BIOKEY_GENTEMPLATE
4.2.7. BIOKEY_VERIFY
4.2.8. BIOKEY_SET_PARAMETER
[函数]
int BIOKEY_GETLASTQUALITY(HANDLE Handle);
[功能]
获取最近⼀次提取模板的模板质量
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
[返回值]
模板质量(0~100)
[备注]
质量不太准确，仅供参考
[函数]
int BIOKEY_GENTEMPLATE(HANDLE Handle, BYTE*Templates[], int TmpCount, BYTE *GTemplate);
[功能]
⽣成登记特征(多个模板之中取最好)
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
Templates[in]
模板数组
TmpCount[in]
模板个数
GTemplate[out]
返回最好的模板(建议分配2048字节)
[返回值]
>0 表⽰成功，值为最好模板的实际数据⻓度
[函数]
int BIOKEY_VERIFY(HANDLE Handle, BYTE*Template1, BYTE *Template2);
[功能]
⽐对两个模板
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
Template1[in]
⽐对模板数据
Template2[in]
⽐对模板数据
[返回值]
返回分数(0~1000), 推荐阈值50
[函数]
int BIOKEY_SET_PARAMETER(HANDLE Handle, int ParameterCode, int ParameterValue);
[功能]
设置参数(不要随意设置)
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
ParameterCode[in]
4.2.9. BIOKEY_DB_ADD
4.2.10. BIOKEY_DB_DEL
4.2.11. BIOKEY_DB_CLEAR
参数代码
1 表⽰设置⽐对阈值
4 表⽰设置旋转⻆度
ParameterValue[in]
参数值
[返回值]
1 表⽰成功
[备注]
除⽐对阈值/旋转⻆度外,其他未说明参数代码不要随意设置
[函数]
int BIOKEY_DB_ADD(HANDLE Handle, int TID, int TempLength, BYTE*Template);
[功能]
添加模板到1:N内存中
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
TID[in]
模板ID(必须>0)
TempLength[in]
模板数据⻓度
Template[in]
登记模板数据
[返回值]
>0 表⽰成功
其他为失败
[备注]
模板只存在内存中，BIOKEY_DB_CLEAR/BIOKEY_CLOSE/程序退出均被释放
[函数]
int BIOKEY_DB_DEL(HANDLE Handle, int TID);
[功能]
从内存中删除⼀个模板
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
TID[in]
模板ID(⻅BIOKEY_DB_ADD)
[返回值]
1 表⽰成功
[函数]
int BIOKEY_DB_CLEAR(HANDLE Handle);
[功能]
清空内存中全部模板
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
[返回值]
1 表⽰成功
4.2.12. BIOKEY_DB_COUNT
4.2.13. BIOKEY_IDENTIFYTEMP
4.2.14. BIOKEY_EXTRACT_GRAYSCALEDATA

5. 附录
5.1 名词解释
[函数]
int BIOKEY_DB_COUNT(HANDLE Handle);
[功能]
获取已添加模板的数量
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
[返回值]
模板数量
[函数]
int BIOKEY_IDENTIFYTEMP(HANDLE Handle, BYTE *Template, int *TID, int *Score);
[功能]
1:N识别
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
Template[in]
指纹模板数据
TID[out]
返回识别成功的指纹ID
Score[out]
返回识别成功的分数(推荐阈值70)
[返回值]
成功返回1
[函数]
int APICALL BIOKEY_EXTRACT_GRAYSCALEDATA(void* Handle, unsigned char* PixelsBuffer, int
width, int height, unsigned char* Template, int maxTmpLen, int PurposeMode);
[功能]
提取特征
[参数]
Handle[in]
算法句柄(⻅BIOKEY_INIT_SIMPLE)
pixelsBuffer[in]
指纹RAW图像数据，⻅sensorCapture
width[in]
图像宽
height[in]
图像⾼
Template[out]
返回指纹模板数据(建议分配2048字节)
PurposeMode[in]
固定传0
[返回值]

> 0 表⽰提取成功，返回模板数据实际⻓度
以下的定义将帮助你理解指纹识别应⽤基本功能,以及帮助快速完成指纹识别应⽤集成开发。
指纹(1:1)⽐对
指纹 （1:1） ⽐对也叫做“指纹验证”,根据⽤⼾ ID 和指纹模板来验证该⽤⼾“是不是”符合其⾝份；或
者⽐对⼀组登记模板和⽐对模板是不是来⾃同⼀个⼿指。
指纹(1:N)⽐对
指纹 （1:N） ⽐对也叫做“指纹辨识”,是指在不知道⽤⼾ ID 的情况下,仅根据输⼊的指纹,在指纹数据
库中进⾏检索,返回符合阈值条件的⽤⼾名、相似度等信息,得出“有没有”该⽤⼾的结论的过程。
