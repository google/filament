// RUN: %dxc -T cs_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability PhysicalStorageBufferAddresses
// CHECK: OpExtension "SPV_KHR_physical_storage_buffer"
// CHECK: OpMemoryModel PhysicalStorageBuffer64 GLSL450
// CHECK-NOT: OpMemberDecorate %S 0 Offset 0
// CHECK-NOT: OpMemberDecorate %S 1 Offset 4
// CHECK-NOT: OpMemberDecorate %S 2 Offset 8
// CHECK-NOT: OpMemberDecorate %S 3 Offset 12
// CHECK: OpMemberDecorate %S_0 0 Offset 0
// CHECK: OpMemberDecorate %S_0 1 Offset 4
// CHECK: OpMemberDecorate %S_0 2 Offset 8
// CHECK-NOT: OpMemberDecorate %S_0 3 Offset 12

// CHECK: %S = OpTypeStruct %uint %uint %uint
// CHECK: %_ptr_Function_S = OpTypePointer Function %S
// CHECK: %S_0 = OpTypeStruct %uint %uint %uint
// CHECK: %_ptr_PhysicalStorageBuffer_S_0 = OpTypePointer PhysicalStorageBuffer %S_0


struct S {
  uint f1;
  uint f2 : 1;
  uint f3 : 3;
  uint f4;
};

uint64_t Address;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  // CHECK: %tmp = OpVariable %_ptr_Function_S Function
  S tmp;

  // CHECK: [[tmp:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tmp %int_0
  // CHECK: OpStore [[tmp]] %uint_2
  tmp.f1 = 2;

  // CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tmp %int_1
  // CHECK: [[tmp_0:%[0-9]+]] = OpLoad %uint [[ptr]]
  // CHECK: [[tmp_1:%[0-9]+]] = OpBitFieldInsert %uint [[tmp_0]] %uint_1 %uint_0 %uint_1
  // CHECK: OpStore [[ptr]] [[tmp_1]]
  tmp.f2 = 1;

  // CHECK: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tmp %int_1
  // CHECK: [[tmp_2:%[0-9]+]] = OpLoad %uint [[ptr_0]]
  // CHECK: [[tmp_3:%[0-9]+]] = OpBitFieldInsert %uint [[tmp_2]] %uint_0 %uint_1 %uint_3
  // CHECK: OpStore [[ptr_0]] [[tmp_3]]
  tmp.f3 = 0;

  // CHECK: [[tmp_4:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tmp %int_2
  // CHECK: OpStore [[tmp_4]] %uint_3
  tmp.f4 = 3;
  vk::RawBufferStore<S>(Address, tmp);
}

