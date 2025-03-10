// RUN: %dxc -enable-16bit-types -fspv-target-env=vulkan1.3 -T cs_6_2 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=int16_t %s | FileCheck %s --check-prefix=CHECK --check-prefix=INTEGERS --check-prefix=INT16
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=int %s | FileCheck %s --check-prefix=CHECK --check-prefix=INTEGERS --check-prefix=INT32
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=int64_t %s | FileCheck %s --check-prefix=CHECK --check-prefix=INTEGERS --check-prefix=INT64
// RUN: %dxc -enable-16bit-types -fspv-target-env=vulkan1.3 -T cs_6_2 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=uint16_t %s | FileCheck %s --check-prefix=CHECK --check-prefix=INTEGERS --check-prefix=UINT16
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=uint %s | FileCheck %s --check-prefix=CHECK --check-prefix=INTEGERS --check-prefix=UINT32
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=uint64_t %s | FileCheck %s --check-prefix=CHECK --check-prefix=INTEGERS --check-prefix=UINT64
// RUN: %dxc -enable-16bit-types -fspv-target-env=vulkan1.3 -T cs_6_2 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=half %s | FileCheck %s --check-prefix=CHECK --check-prefix=FLOATS --check-prefix=HALF-ENABLED
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=half %s | FileCheck %s --check-prefix=CHECK --check-prefix=FLOATS --check-prefix=HALF-DISABLED
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=float %s | FileCheck %s --check-prefix=CHECK --check-prefix=FLOATS --check-prefix=FLOAT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=double %s | FileCheck %s --check-prefix=CHECK --check-prefix=FLOATS --check-prefix=DOUBLE

#include "vk/khr/cooperative_matrix.h"

StructuredBuffer<float> structured_buffer;

RWStructuredBuffer<TYPE> data;

// CHECK: OpCapability CooperativeMatrixKHR
// CHECK: OpExtension "SPV_KHR_cooperative_matrix"

// Check that the type is correctly created.
// INT16: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %short %uint_3 %uint_16 %uint_8 %uint_0
// INT32: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_16 %uint_8 %uint_0
// INT64: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %long %uint_3 %uint_16 %uint_8 %uint_0
// UINT16: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %ushort %uint_3 %uint_16 %uint_8 %uint_0
// UINT32: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %uint %uint_3 %uint_16 %uint_8 %uint_0
// UINT64: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %ulong %uint_3 %uint_16 %uint_8 %uint_0

// When 16bit types are not enabled, HALF is a float
// HALF-DISABLED: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_16 %uint_8 %uint_0
// HALF-ENABLED: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %half %uint_3 %uint_16 %uint_8 %uint_0
// FLOAT: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_16 %uint_8 %uint_0
// DOUBLE: %spirvIntrinsicType = OpTypeCooperativeMatrixKHR %double %uint_3 %uint_16 %uint_8 %uint_0

[numthreads(64, 1, 1)] void main() {
  using CoopMat = vk::khr::CooperativeMatrixA<
      TYPE, vk::ScopeSubgroup, 16, 8>;

  // CHECK: [[ac1:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_{{.*}} %data %int_0 %uint_0
  // CHECK: [[m:%[0-9]+]] = OpCooperativeMatrixLoadKHR %spirvIntrinsicType [[ac1]] %int_1 %uint_64 None
  CoopMat m = CoopMat::Load<vk::CooperativeMatrixLayoutColumnMajorKHR>(data, 0, 64);

  // CHECK: [[len:%[0-9]+]] = OpCooperativeMatrixLengthKHR %uint %spirvIntrinsicType
  // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_{{.*}} %structured_buffer %int_0 [[len]]
  // CHECK: [[n:%[0-9]+]] = OpCooperativeMatrixLoadKHR %spirvIntrinsicType [[ac]] %int_0 %uint_64 None
  uint32_t length = CoopMat::GetLength();
  CoopMat n = CoopMat::Load<vk::CooperativeMatrixLayoutRowMajorKHR>(structured_buffer, length, 64);

  // INTEGERS: [[r:%[0-9]+]] = OpIAdd %spirvIntrinsicType [[m]] [[n]]
  // FLOATS: [[r:%[0-9]+]] = OpFAdd %spirvIntrinsicType [[m]] [[n]]
  CoopMat r = m + n;

  // INTEGERS: [[n:%[0-9]+]] = OpISub %spirvIntrinsicType [[m]] [[r]]
  // FLOATS: [[n:%[0-9]+]] = OpFSub %spirvIntrinsicType [[m]] [[r]]
  n = m - r;

  // INTEGERS: [[m:%[0-9]+]] = OpSNegate %spirvIntrinsicType [[n]]
  // FLOATS: [[m:%[0-9]+]] = OpFNegate %spirvIntrinsicType [[n]]
  m = n.negate();

  // INT16: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %short_2
  // INT32: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %int_2
  // INT64: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %long_2
  // UINT16: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %ushort_2
  // UINT32: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %uint_2
  // UINT64: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %ulong_2
  // HALF-DISABLED: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %float_2
  // HALF-ENABLED: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %half_0x1p_1
  // FLOAT: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %float_2
  // DOUBLE: [[r:%[0-9]+]] = OpMatrixTimesScalar %spirvIntrinsicType [[m]] %double_2
  r = m * 2.0;

  // INT16: [[n:%[0-9]+]] = OpSDiv %spirvIntrinsicType [[r]] [[m]]
  // INT32: [[n:%[0-9]+]] = OpSDiv %spirvIntrinsicType [[r]] [[m]]
  // INT64: [[n:%[0-9]+]] = OpSDiv %spirvIntrinsicType [[r]] [[m]]
  // UINT16: [[n:%[0-9]+]] = OpUDiv %spirvIntrinsicType [[r]] [[m]]
  // UINT32: [[n:%[0-9]+]] = OpUDiv %spirvIntrinsicType [[r]] [[m]]
  // UINT64: [[n:%[0-9]+]] = OpUDiv %spirvIntrinsicType [[r]] [[m]]
  // HALF-DISABLED: [[n:%[0-9]+]] = OpFDiv %spirvIntrinsicType [[r]] [[m]]
  // HALF-ENABLED: [[n:%[0-9]+]] = OpFDiv %spirvIntrinsicType [[r]] [[m]]
  // FLOAT: [[n:%[0-9]+]] = OpFDiv %spirvIntrinsicType [[r]] [[m]]
  // DOUBLE: [[n:%[0-9]+]] = OpFDiv %spirvIntrinsicType [[r]] [[m]]
  n = r / m;

  // INTEGERS: [[r:%[0-9]+]] = OpIMul %spirvIntrinsicType [[n]] [[m]]
  // FLOATS: [[r:%[0-9]+]] = OpFMul %spirvIntrinsicType [[n]] [[m]]
  r = n * m;

  // CHECK: OpCooperativeMatrixStoreKHR [[ac1]] [[r]] %int_0 %uint_64 None
  r.Store<vk::CooperativeMatrixLayoutRowMajorKHR>(data, 0, 64);

  // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_{{.*}} %data %int_0 %uint_16
  // CHECK: OpCooperativeMatrixStoreKHR [[ac]] [[r]] %int_1 %uint_64 None
  r.Store<vk::CooperativeMatrixLayoutColumnMajorKHR>(data, 16, 64);
}
