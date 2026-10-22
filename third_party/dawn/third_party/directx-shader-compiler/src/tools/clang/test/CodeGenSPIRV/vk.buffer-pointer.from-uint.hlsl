// RUN: %dxc -spirv -Od -T cs_6_7 %s | FileCheck %s
// RUN: %dxc -spirv -Od -T cs_6_7 -DALIGN_16 %s | FileCheck %s
// RUN: %dxc -spirv -Od -T cs_6_7 -DNO_PC %s | FileCheck %s

// Was getting bogus type errors with the defined changes

#ifdef ALIGN_16
typedef vk::BufferPointer<uint, 16> BufferType;
#else
typedef vk::BufferPointer<uint, 32> BufferType;
#endif
#ifndef NO_PC
struct PushConstantStruct {
  BufferType push_buffer;
};
[[vk::push_constant]] PushConstantStruct push_constant;
#endif

RWStructuredBuffer<uint> output;

// CHECK: [[INT:%[_0-9A-Za-z]*]] = OpTypeInt 32 1
// CHECK: [[I0:%[_0-9A-Za-z]*]] = OpConstant [[INT]] 0
// CHECK: [[UINT:%[_0-9A-Za-z]*]] = OpTypeInt 32 0
// CHECK: [[U0:%[_0-9A-Za-z]*]] = OpConstant [[UINT]] 0
// CHECK: [[PPUINT:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[UINT]]
// CHECK: [[PFPPUINT:%[_0-9A-Za-z]*]] = OpTypePointer Function [[PPUINT]]
// CHECK: [[PUUINT:%[_0-9A-Za-z]*]] = OpTypePointer Uniform [[UINT]]
// CHECK: [[OUTPUT:%[_0-9A-Za-z]*]] = OpVariable %{{[_0-9A-Za-z]*}} Uniform

[numthreads(1, 1, 1)]
void main() {
  uint64_t addr = 123;
  vk::BufferPointer<uint, 32> test = vk::BufferPointer<uint, 32>(addr);
  output[0] = test.Get();
}

// CHECK: [[TEST:%[_0-9A-Za-z]*]] = OpVariable [[PFPPUINT]] Function
// CHECK: [[X1:%[_0-9A-Za-z]*]] = OpConvertUToPtr [[PPUINT]]
// CHECK: OpStore [[TEST]] [[X1]]
// CHECK: [[X2:%[_0-9A-Za-z]*]] = OpLoad [[PPUINT]] [[TEST]]
// CHECK: [[X3:%[_0-9A-Za-z]*]] = OpLoad [[UINT]] [[X2]] Aligned 32
// CHECK: [[X4:%[_0-9A-Za-z]*]] = OpAccessChain [[PUUINT]] [[OUTPUT]] [[I0]] [[U0]]
// CHECK: OpStore [[X4]] [[X3]]
// CHECK: OpReturn
// CHECK: OpFunctionEnd

