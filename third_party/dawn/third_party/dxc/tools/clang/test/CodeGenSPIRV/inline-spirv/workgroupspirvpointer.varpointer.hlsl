// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers %s 2>&1 | FileCheck %s

#include "vk/spirv.h"

// CHECK: OpCapability VariablePointers

RWStructuredBuffer<int> data;

groupshared int shared_data[64];

[[vk::ext_instruction(/* OpLoad */ 61)]] int
Load(vk::WorkgroupSpirvPointer<int> p);

[[noinline]]
int foo(vk::WorkgroupSpirvPointer<int> param) {
  return Load(param);
}

[[vk::ext_capability(/* VariablePointersCapability */ 4442)]]
[numthreads(64, 1, 1)] void main() {
  vk::WorkgroupSpirvPointer<int> p = vk::GetGroupSharedAddress(shared_data[0]);
  data[0] = foo(p);
}
