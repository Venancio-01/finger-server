import ref from 'ref-napi'
import ArrayType from 'ref-array-di'
import StructType from 'ref-struct-di'

export const CArray = ArrayType(ref)
export const CStruct = StructType(ref)

// 设备数组的结构体类型
export const DeviceType = CStruct({
  vid: ref.types.ushort,
  pid: ref.types.ushort,
  szSerialNumber: CArray(ref.types.uchar, 64),
  bus_number: ref.types.uint32,
  device_address: ref.types.uint32,
  extraPtr: ref.types.uint,
})

// 句柄类型
export const HandleType = ref.refType(ref.types.uint)
// uchar 类型
export const UcharType = CArray(ref.types.uchar)
// int 类型
export const IntType = CArray(ref.types.int)
// 设备结构体数组
export const DeviceArrayType = CArray(DeviceType)
// 设备结构体指针
export const DeviceTypePointerType = ref.refType(DeviceType)
// uchar 数组类型
export const UcharArrayType = CArray(CArray(ref.types.uchar))

export const TemplateType = CArray(UcharType)
