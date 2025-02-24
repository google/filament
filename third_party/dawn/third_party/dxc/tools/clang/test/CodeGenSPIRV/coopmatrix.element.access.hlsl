// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers %s | FileCheck %s

#include "vk/khr/cooperative_matrix.h"

RWStructuredBuffer<int> data;
int stride;

// CHECK: OpCapability CooperativeMatrixKHR
// CHECK: OpExtension "SPV_KHR_cooperative_matrix"

// CHECK-DAG: [[typeA:%spirvIntrinsicType[_0-9]*]] = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_16 %uint_4 %uint_0

[numthreads(64, 1, 1)] void main() {
  using IntMatA = vk::khr::CooperativeMatrixA<int, vk::ScopeSubgroup, 16, 4>;

  // CHECK: [[a:%[0-9]+]] = OpVariable %_ptr_Function_spirvIntrinsicType Function
  // CHECK: [[v:%[0-9]+]] = OpCompositeConstruct %spirvIntrinsicType %int_10
  // CHECK: OpStore [[a]] [[v]]
  IntMatA a = IntMatA::Splat(10);

  uint32_t length = a.GetLength();
  // CHECK: OpLoopMerge [[mbb:%[0-9]+]]
  for (int i = 0; i < length; ++i) {
    // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[a]]
    // CHECK: [[get:%[0-9]+]] = OpLoad %int [[ac]]
    // CHECK: [[add:%[0-9]+]] = OpIAdd %int [[get]] %int_1
    // CHECK: OpStore [[ac]] [[add]]
    int v = a.Get(i);
    a.Set(v + 1, i);
  }
  // CHECK: [[mbb]] = OpLabel
  a.Store<vk::CooperativeMatrixLayoutRowMajorKHR>(data, 64, stride);
}
