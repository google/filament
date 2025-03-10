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

// CHECK: %temp_var_S = OpVariable %_ptr_Function_S Function
// CHECK: %temp_var_S_0 = OpVariable %_ptr_Function_S Function
// CHECK: %temp_var_S_1 = OpVariable %_ptr_Function_S Function
// CHECK: %temp_var_S_2 = OpVariable %_ptr_Function_S Function

struct S {
  uint f1;
  uint f2 : 1;
  uint f3 : 1;
  uint f4;
};

uint64_t Address;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {


  {
    // CHECK: [[tmp:%[0-9]+]] = OpAccessChain %_ptr_Uniform_ulong %_Globals %int_0
    // CHECK: [[tmp_0:%[0-9]+]] = OpLoad %ulong [[tmp]]
    // CHECK: [[ptr:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_S_0 [[tmp_0]]
    // CHECK: [[tmp_1:%[0-9]+]] = OpLoad %S_0 [[ptr]] Aligned 4
    // CHECK: [[member0:%[0-9]+]] = OpCompositeExtract %uint [[tmp_1]] 0
    // CHECK: [[member1:%[0-9]+]] = OpCompositeExtract %uint [[tmp_1]] 1
    // CHECK: [[member2:%[0-9]+]] = OpCompositeExtract %uint [[tmp_1]] 2
    // CHECK: [[tmp_2:%[0-9]+]] = OpCompositeConstruct %S [[member0]] [[member1]] [[member2]]
    // CHECK: OpStore %temp_var_S [[tmp_2]]
    // CHECK: [[tmp_3:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %temp_var_S %int_0
    // CHECK: [[tmp_4:%[0-9]+]] = OpLoad %uint [[tmp_3]]
    // CHECK: OpStore %tmp1 [[tmp_4]]
    uint tmp1 = vk::RawBufferLoad<S>(Address).f1;
  }

  {
    // CHECK: [[tmp_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_ulong %_Globals %int_0
    // CHECK: [[tmp_6:%[0-9]+]] = OpLoad %ulong [[tmp_5]]
    // CHECK: [[ptr_0:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_S_0 [[tmp_6]]
    // CHECK: [[tmp_7:%[0-9]+]] = OpLoad %S_0 [[ptr_0]] Aligned 4
    // CHECK: [[member0_0:%[0-9]+]] = OpCompositeExtract %uint [[tmp_7]] 0
    // CHECK: [[member1_0:%[0-9]+]] = OpCompositeExtract %uint [[tmp_7]] 1
    // CHECK: [[member2_0:%[0-9]+]] = OpCompositeExtract %uint [[tmp_7]] 2
    // CHECK: [[tmp_8:%[0-9]+]] = OpCompositeConstruct %S [[member0_0]] [[member1_0]] [[member2_0]]
    // CHECK: OpStore %temp_var_S_0 [[tmp_8]]
    // CHECK: [[tmp_9:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %temp_var_S_0 %int_1
    // CHECK: [[tmp_10:%[0-9]+]] = OpLoad %uint [[tmp_9]]
    // CHECK: [[tmp_11:%[0-9]+]] = OpBitFieldUExtract %uint [[tmp_10]] %uint_0 %uint_1
    // CHECK: OpStore %tmp2 [[tmp_11]]
    uint tmp2 = vk::RawBufferLoad<S>(Address).f2;
  }

  {
    // CHECK: [[tmp_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_ulong %_Globals %int_0
    // CHECK: [[tmp_13:%[0-9]+]] = OpLoad %ulong [[tmp_12]]
    // CHECK: [[ptr_1:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_S_0 [[tmp_13]]
    // CHECK: [[tmp_14:%[0-9]+]] = OpLoad %S_0 [[ptr_1]] Aligned 4
    // CHECK: [[member0_1:%[0-9]+]] = OpCompositeExtract %uint [[tmp_14]] 0
    // CHECK: [[member1_1:%[0-9]+]] = OpCompositeExtract %uint [[tmp_14]] 1
    // CHECK: [[member2_1:%[0-9]+]] = OpCompositeExtract %uint [[tmp_14]] 2
    // CHECK: [[tmp_15:%[0-9]+]] = OpCompositeConstruct %S [[member0_1]] [[member1_1]] [[member2_1]]
    // CHECK: OpStore %temp_var_S_1 [[tmp_15]]
    // CHECK: [[tmp_16:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %temp_var_S_1 %int_1
    // CHECK: [[tmp_17:%[0-9]+]] = OpLoad %uint [[tmp_16]]
    // CHECK: [[tmp_18:%[0-9]+]] = OpBitFieldUExtract %uint [[tmp_17]] %uint_1 %uint_1
    // CHECK: OpStore %tmp3 [[tmp_18]]
    uint tmp3 = vk::RawBufferLoad<S>(Address).f3;
  }

  {
    // CHECK: [[tmp_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_ulong %_Globals %int_0
    // CHECK: [[tmp_20:%[0-9]+]] = OpLoad %ulong [[tmp_19]]
    // CHECK: [[ptr_2:%[0-9]+]] = OpBitcast %_ptr_PhysicalStorageBuffer_S_0 [[tmp_20]]
    // CHECK: [[tmp_21:%[0-9]+]] = OpLoad %S_0 [[ptr_2]] Aligned 4
    // CHECK: [[member0_2:%[0-9]+]] = OpCompositeExtract %uint [[tmp_21]] 0
    // CHECK: [[member1_2:%[0-9]+]] = OpCompositeExtract %uint [[tmp_21]] 1
    // CHECK: [[member2_2:%[0-9]+]] = OpCompositeExtract %uint [[tmp_21]] 2
    // CHECK: [[tmp_22:%[0-9]+]] = OpCompositeConstruct %S [[member0_2]] [[member1_2]] [[member2_2]]
    // CHECK: OpStore %temp_var_S_2 [[tmp_22]]
    // CHECK: [[tmp_23:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %temp_var_S_2 %int_2
    // CHECK: [[tmp_24:%[0-9]+]] = OpLoad %uint [[tmp_23]]
    // CHECK: OpStore %tmp4 [[tmp_24]]
    uint tmp4 = vk::RawBufferLoad<S>(Address).f4;
  }
}

