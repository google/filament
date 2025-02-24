// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers %s 2>&1 | FileCheck %s

#include "vk/spirv.h"

// CHECK-NOT: OpCapability VariablePointers

RWStructuredBuffer<int> data;

groupshared int shared_data[64];

[[vk::ext_instruction(/* OpLoad */ 61)]] int
Load(vk::WorkgroupSpirvPointer<int> p);

int foo(vk::WorkgroupSpirvPointer<int> param) {
  return Load(param);
}

[numthreads(64, 1, 1)] void main() {
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Workgroup_int %shared_data %int_0
// CHECK: [[ld:%[0-9]+]] = OpLoad %int [[ac]]
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_int %data %int_0 %uint_0
// CHECK: OpStore [[ac]] [[ld]]

  vk::WorkgroupSpirvPointer<int> p = vk::GetGroupSharedAddress(shared_data[0]);
  data[0] = foo(p);
}
