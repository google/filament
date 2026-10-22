// RUN: %dxc -T ps_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.3

// CHECK: OpCapability GroupNonUniform

// CHECK: OpEntryPoint Fragment
// CHECK-SAME: %SubgroupSize

// CHECK: OpDecorate %SubgroupSize BuiltIn SubgroupSize
// CHECK: OpDecorate %SubgroupSize Flat

// CHECK: %SubgroupSize = OpVariable %_ptr_Input_uint Input

float4 main() : SV_Target {
// CHECK: OpLoad %uint %SubgroupSize
    uint laneCount = WaveGetLaneCount();
    return float4(laneCount, 1, 1, 1);
}
