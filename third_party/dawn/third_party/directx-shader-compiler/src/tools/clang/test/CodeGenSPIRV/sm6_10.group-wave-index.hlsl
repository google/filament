// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.3

RWStructuredBuffer<uint> output: register(u0);

// CHECK: OpCapability GroupNonUniform

// CHECK: OpEntryPoint GLCompute
// CHECK-SAME: %SubgroupId

// CHECK: OpDecorate %SubgroupId BuiltIn SubgroupId

// CHECK: %SubgroupId = OpVariable %_ptr_Input_uint Input

[numthreads(64, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    // CHECK: OpLoad %uint %SubgroupId
    output[id.x] = GetGroupWaveIndex();
}
