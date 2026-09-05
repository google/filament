// RUN: %dxc -T cs_6_6 -E main -fcgl -spirv %s | FileCheck %s

RWStructuredBuffer<uint> buffer;
groupshared uint value;

struct S1 {
  uint m0;
};

struct S2 {
  S1 m0[2];
};

RWStructuredBuffer<uint> returnBuffer() {
  return buffer;
}

void passBuffer(RWStructuredBuffer<uint> param) {
  InterlockedAdd(value, 1, param[0]);
}

[numthreads(1, 1, 1)]
void main() {
  uint result;
  InterlockedAdd(value, 1, result);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK:                   OpStore %result [[tmp]]

  uint2 value2;
  InterlockedAdd(value, 1, value2.x);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %value2 %int_0
// CHECK:                   OpStore [[ptr]] [[tmp]]

  S1 s1;
  InterlockedAdd(value, 1, s1.m0);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s1 %int_0
// CHECK:                   OpStore [[ptr]] [[tmp]]

  uint array[2];
  InterlockedAdd(value, 1, array[0]);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %array %int_0
// CHECK:                   OpStore [[ptr]] [[tmp]]

  S2 s2;
  InterlockedAdd(value, 1, s2.m0[1].m0);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s2 %int_0 %int_1 %int_0
// CHECK:                   OpStore [[ptr]] [[tmp]]

  InterlockedAdd(value, 1, buffer[0]);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buffer %int_0 %uint_0
// CHECK:                   OpStore [[ptr]] [[tmp]]

  InterlockedAdd(value, 1, returnBuffer()[0]);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK: [[buf:%[0-9]+]] = OpFunctionCall %_ptr_Uniform_type_RWStructuredBuffer_uint %returnBuffer
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[buf]] %int_0 %uint_0
// CHECK:                   OpStore [[ptr]] [[tmp]]

  passBuffer(buffer);
// CHECK: [[tmp:%[0-9]+]] = OpAtomicIAdd %uint %value %uint_2 %uint_0 %uint_1
// CHECK: [[buf:%[0-9]+]] = OpLoad %_ptr_Uniform_type_RWStructuredBuffer_uint %param
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[buf]] %int_0 %uint_0
// CHECK:                   OpStore [[ptr]] [[tmp]]
}
