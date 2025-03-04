// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.5

RWStructuredBuffer<uint> values;

// CHECK: OpCapability GroupNonUniform

// CHECK: OpEntryPoint GLCompute
// CHECK-SAME: %SubgroupSize

// CHECK: OpDecorate %SubgroupSize BuiltIn SubgroupSize

// CHECK: %SubgroupSize = OpVariable %_ptr_Input_uint Input

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
// CHECK: OpLoad %uint %SubgroupSize
    values[id.x] = WaveGetLaneCount();
}
