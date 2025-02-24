// RUN: not %dxc -fspv-target-env=vulkan1.1 -T cs_6_0 -E main -spirv -HV 2021 -I %hlsl_headers -DTYPE=int %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=INTEGERS --check-prefix=INT32

#include "vk/khr/cooperative_matrix.h"

[numthreads(64, 1, 1)] void main() {
}

// CHECK: error: "CooperativeMatrix requires a minimum of SPIR-V 1.6"
