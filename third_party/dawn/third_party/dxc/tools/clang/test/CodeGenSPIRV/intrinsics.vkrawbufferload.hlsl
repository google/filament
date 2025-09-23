// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability PhysicalStorageBufferAddresses
// CHECK: OpExtension "SPV_KHR_physical_storage_buffer"
// CHECK: OpMemoryModel PhysicalStorageBuffer64 GLSL450

// CHECK:                 OpMemberDecorate %BufferData_0 0 Offset 0
// CHECK:                 OpMemberDecorate %BufferData_0 1 Offset 4
// CHECK: %BufferData_0 = OpTypeStruct %float %v3float
struct BufferData {
  float  f;
  float3 v;
};

using MyInt = vk::SpirvType<
    /*spv::OpTypeInt*/21,
    1,1, // size and alignment
    vk::Literal<vk::integral_constant<uint,16> >, // bits
    vk::Literal<vk::integral_constant<uint,1> > // signed
>;

uint64_t Address;

[[vk::ext_capability(/* Int16 */ 22)]]
float4 main() : SV_Target0 {
  // CHECK:      [[addr:%[0-9]+]] = OpLoad %ulong
  // CHECK-NEXT: [[buf:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_float [[addr]]
  // CHECK-NEXT: [[load:%[0-9]+]] = OpLoad %float [[buf]] Aligned 4
  // CHECK-NEXT: OpStore %x [[load]]
  float x = vk::RawBufferLoad<float>(Address);

  // CHECK:      [[addr_0:%[0-9]+]] = OpLoad %ulong
  // CHECK-NEXT: [[buf_0:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_double [[addr_0]]
  // CHECK-NEXT: [[load_0:%[0-9]+]] = OpLoad %double [[buf_0]] Aligned 8
  // CHECK-NEXT: OpStore %y [[load_0]]
  double y = vk::RawBufferLoad<double>(Address, 8);

  // CHECK:      [[buf_1:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_uint
  // CHECK-NEXT: [[load_1:%[0-9]+]] = OpLoad %uint [[buf_1]] Aligned 4
  // CHECK-NEXT: [[z:%[0-9]+]] = OpINotEqual %bool [[load_1]] %uint_0
  // CHECK-NEXT: OpStore %z [[z]]
  bool z = vk::RawBufferLoad<bool>(Address, 4);

  // CHECK:      [[buf_2:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_v2float
  // CHECK-NEXT: [[load_2:%[0-9]+]] = OpLoad %v2float [[buf_2]] Aligned 8
  // CHECK-NEXT: OpStore %w [[load_2]]
  float2 w = vk::RawBufferLoad<float2>(Address, 8);

  // CHECK:      [[buf_3:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_uint
  // CHECK-NEXT: [[load_3:%[0-9]+]] = OpLoad %uint [[buf_3]] Aligned 4
  // CHECK-NEXT: OpStore %v [[load_3]]
  uint v = vk::RawBufferLoad(Address);

  // CHECK: [[buf:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_BufferData_0
  // CHECK-NEXT: [[load:%[0-9]+]] = OpLoad %BufferData_0 [[buf]] Aligned 4
  BufferData d = vk::RawBufferLoad<BufferData>(Address);

  // CHECK: [[buf:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_BufferData_0 %ulong_0
  // CHECK-NEXT: [[load:%[0-9]+]] = OpLoad %BufferData_0 [[buf]] Aligned 4
  d = vk::RawBufferLoad<BufferData>(0);

  // CHECK: [[buf:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_spirvIntrinsicType %ulong_0
  // CHECK-NEXT: [[load:%[0-9]+]] = OpLoad %spirvIntrinsicType [[buf]] Aligned 4
  // CHECK-NEXT: OpStore %mi [[load]]
  MyInt mi = vk::RawBufferLoad<MyInt>(0);

  return float4(w.x, x, y, z);
}
