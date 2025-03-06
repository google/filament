// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers %s | FileCheck %s

#include "vk/khr/cooperative_matrix.h"

RWStructuredBuffer<int> data;
uint stride;

// CHECK: OpCapability CooperativeMatrixKHR
// CHECK: OpExtension "SPV_KHR_cooperative_matrix"

// CHECK-DAG: [[typeA:%spirvIntrinsicType[_0-9]*]] = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_16 %uint_4 %uint_0
// CHECK-DAG: [[typeB:%spirvIntrinsicType[_0-9]*]] = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_4 %uint_8 %uint_1
// CHECK-DAG: [[typeAc:%spirvIntrinsicType[_0-9]*]] = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_16 %uint_8 %uint_2

// CHECK: [[r:%[0-9]+]] = OpUndef [[typeAc]]
[numthreads(64, 1, 1)] void main() {
  using IntMatA = vk::khr::CooperativeMatrixA<int, vk::ScopeSubgroup, 16, 4>;
  using IntMatB = vk::khr::CooperativeMatrixB<int, vk::ScopeSubgroup, 4, 8>;
  using IntMatAc = vk::khr::CooperativeMatrixAccumulator<int, vk::ScopeSubgroup, 16, 8>;

  // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %_Globals %int_0
  // CHECK: [[stride:%[0-9]+]] = OpLoad %uint [[ac]]

  // CHECK: [[a:%[0-9]+]] = OpCooperativeMatrixLoadKHR [[typeA]] {{%[0-9]*}} %int_1 [[stride]] None
  IntMatA a = IntMatA::Load<vk::CooperativeMatrixLayoutColumnMajorKHR>(data, 0, stride);

  // CHECK: [[b:%[0-9]+]] = OpCooperativeMatrixLoadKHR [[typeB]] {{%[0-9]*}} %int_0 [[stride]] None
  IntMatB b = IntMatB::Load<vk::CooperativeMatrixLayoutRowMajorKHR>(data, 32, stride);

  // TODO: Is default initialization meaningful?
  IntMatAc r;

  // CHECK: [[r2:%[0-9]+]] = OpCooperativeMatrixMulAddKHR [[typeAc]] [[a]] [[b]] [[r]] MatrixASignedComponentsKHR|MatrixBSignedComponentsKHR|MatrixCSignedComponentsKHR|MatrixResultSignedComponentsKHR
  r = cooperativeMatrixMultiplyAdd(a, b, r);

  // CHECK: [[r:%[0-9]+]] = OpCooperativeMatrixMulAddKHR [[typeAc]] [[a]] [[b]] [[r2]] MatrixASignedComponentsKHR|MatrixBSignedComponentsKHR|MatrixCSignedComponentsKHR|MatrixResultSignedComponentsKHR|SaturatingAccumulationKHR
  r = cooperativeMatrixSaturatingMultiplyAdd(a, b, r);

  // CHECK: OpCooperativeMatrixStoreKHR {{.*}} [[r]] %int_0 [[stride]] None
  r.Store<vk::CooperativeMatrixLayoutRowMajorKHR>(data, 64, stride);
}
