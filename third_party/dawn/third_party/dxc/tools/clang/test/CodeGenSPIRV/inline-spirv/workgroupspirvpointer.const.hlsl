// RUN: not %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers %s 2>&1 | FileCheck %s

#include "vk/khr/cooperative_matrix.h"

RWStructuredBuffer<int> data;

groupshared int shared_data[64];

[numthreads(64, 1, 1)] void main() {
  vk::WorkgroupSpirvPointer<int> p = vk::GetGroupSharedAddress(shared_data[0]);
  if (data[0] > 10 ) {
    p = vk::GetGroupSharedAddress(shared_data[0]);
  }
}

// CHECK:  cannot assign to variable 'p' with const-qualified type
