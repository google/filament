// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers %s | FileCheck %s

#include "vk/khr/cooperative_matrix.h"

RWStructuredBuffer<int> data;
int stride;

// CHECK: OpCapability CooperativeMatrixKHR
// CHECK: OpExtension "SPV_KHR_cooperative_matrix"

[numthreads(64, 1, 1)] void main() {
  using IntMatA = vk::khr::CooperativeMatrixA<int, vk::ScopeSubgroup, 16, 4>;
  using FloatMatA = vk::khr::CooperativeMatrixA<float, vk::ScopeSubgroup, 16, 4>;

  // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_int %data %int_0 %uint_0
  // CHECK: [[ld:%[0-9]+]] = OpCooperativeMatrixLoadKHR %spirvIntrinsicType [[ac]] %int_1
  IntMatA int_matrix = IntMatA::Load<vk::CooperativeMatrixLayoutColumnMajorKHR>(data, 0, stride);

  // CHECK: [[result:%[0-9]+]] = OpConvertSToF %spirvIntrinsicType_0 [[ld]]
  FloatMatA float_matrix = int_matrix.cast<float>();

  // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_int %data %int_0 %uint_64
  // CHECK: OpCooperativeMatrixStoreKHR [[ac]] [[result]] %int_0
  float_matrix.Store<vk::CooperativeMatrixLayoutRowMajorKHR>(data, 64, stride);
}
