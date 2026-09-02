// RUN: %dxc -T ps_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: ; Version: 1.3

// CHECK: OpCapability GroupNonUniform

// CHECK: OpEntryPoint Fragment
// CHECK-SAME: %SubgroupLocalInvocationId

// CHECK: OpDecorate %SubgroupLocalInvocationId BuiltIn SubgroupLocalInvocationId
// CHECK: OpDecorate %SubgroupLocalInvocationId Flat

// CHECK: %SubgroupLocalInvocationId = OpVariable %_ptr_Input_uint Input

float4 main() : SV_Target {
// CHECK: OpLoad %uint %SubgroupLocalInvocationId
    uint laneIndex = WaveGetLaneIndex();
    return float4(laneIndex, 1, 1, 1);
}
