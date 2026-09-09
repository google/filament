// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

ByteAddressBuffer myBuffer;

struct S {
  uint32_t x : 8;
  uint32_t y : 8;
};

[numthreads(1, 1, 1)]
void main() {
  uint addr = 0;

// CHECK: [[addr1:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[word_addr:%[0-9]+]] = OpShiftRightLogical %uint [[addr1]] %uint_2
// CHECK-NEXT: [[load_ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[word_addr]]
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %uint [[load_ptr]]
  uint word = myBuffer.Load(addr);

// CHECK: [[addr3:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[load2_word0Addr:%[0-9]+]] = OpShiftRightLogical %uint [[addr3]] %uint_2
// CHECK-NEXT: [[load_ptr10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load2_word0Addr]]
// CHECK-NEXT: [[load2_word0:%[0-9]+]] = OpLoad %uint [[load_ptr10]]
// CHECK-NEXT: [[load2_word1Addr:%[0-9]+]] = OpIAdd %uint [[load2_word0Addr]] %uint_1
// CHECK-NEXT: [[load_ptr11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load2_word1Addr]]
// CHECK-NEXT: [[load2_word1:%[0-9]+]] = OpLoad %uint [[load_ptr11]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %v2uint [[load2_word0]] [[load2_word1]]
  uint2 word2 = myBuffer.Load2(addr);

// CHECK: [[addr2:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[load3_word0Addr:%[0-9]+]] = OpShiftRightLogical %uint [[addr2]] %uint_2
// CHECK-NEXT: [[load_ptr7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load3_word0Addr]]
// CHECK-NEXT: [[load3_word0:%[0-9]+]] = OpLoad %uint [[load_ptr7]]
// CHECK-NEXT: [[load3_word1Addr:%[0-9]+]] = OpIAdd %uint [[load3_word0Addr]] %uint_1
// CHECK-NEXT: [[load_ptr8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load3_word1Addr]]
// CHECK-NEXT: [[load3_word1:%[0-9]+]] = OpLoad %uint [[load_ptr8]]
// CHECK-NEXT: [[load3_word2Addr:%[0-9]+]] = OpIAdd %uint [[load3_word0Addr]] %uint_2
// CHECK-NEXT: [[load_ptr9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load3_word2Addr]]
// CHECK-NEXT: [[load3_word2:%[0-9]+]] = OpLoad %uint [[load_ptr9]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %v3uint [[load3_word0]] [[load3_word1]] [[load3_word2]]
  uint3 word3 = myBuffer.Load3(addr);

// CHECK: [[addr:%[0-9]+]] = OpLoad %uint %addr
// CHECK-NEXT: [[load4_word0Addr:%[0-9]+]] = OpShiftRightLogical %uint [[addr]] %uint_2
// CHECK-NEXT: [[load_ptr3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load4_word0Addr]]
// CHECK-NEXT: [[load4_word0:%[0-9]+]] = OpLoad %uint [[load_ptr3]]
// CHECK-NEXT: [[load4_word1Addr:%[0-9]+]] = OpIAdd %uint [[load4_word0Addr]] %uint_1
// CHECK-NEXT: [[load_ptr4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load4_word1Addr]]
// CHECK-NEXT: [[load4_word1:%[0-9]+]] = OpLoad %uint [[load_ptr4]]
// CHECK-NEXT: [[load4_word2Addr:%[0-9]+]] = OpIAdd %uint [[load4_word0Addr]] %uint_2
// CHECK-NEXT: [[load_ptr5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load4_word2Addr]]
// CHECK-NEXT: [[load4_word2:%[0-9]+]] = OpLoad %uint [[load_ptr5]]
// CHECK-NEXT: [[load4_word3Addr:%[0-9]+]] = OpIAdd %uint [[load4_word0Addr]] %uint_3
// CHECK-NEXT: [[load_ptr6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[load4_word3Addr]]
// CHECK-NEXT: [[load4_word3:%[0-9]+]] = OpLoad %uint [[load_ptr6]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %v4uint [[load4_word0]] [[load4_word1]] [[load4_word2]] [[load4_word3]]
  uint4 word4 = myBuffer.Load4(addr);

// CHECK: [[idx:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %myBuffer %uint_0 [[idx]]
// CHECK: [[bitfield:%[0-9]+]] = OpLoad %uint [[ac]]
// CHECK: [[s:%[0-9]+]] = OpCompositeConstruct %S [[bitfield]]
// CHECK: OpStore %s [[s]]
  S s = myBuffer.Load<S>(0);
}
