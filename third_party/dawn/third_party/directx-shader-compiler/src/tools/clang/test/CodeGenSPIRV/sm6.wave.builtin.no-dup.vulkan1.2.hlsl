// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// Some wave ops translates into SPIR-V builtin variables.
// Test that we are not generating duplicated builtins for multiple calls of
// of the same wave ops.
RWStructuredBuffer<uint> values;

// CHECK: OpEntryPoint GLCompute
// CHECK-DAG: %SubgroupSize
// CHECK-DAG: %SubgroupLocalInvocationId

// CHECK: OpDecorate %SubgroupSize BuiltIn SubgroupSize
// CHECK-NOT: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn SubgroupSize

// CHECK: OpDecorate %SubgroupLocalInvocationId BuiltIn SubgroupLocalInvocationId
// CHECK-NOT: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn SubgroupLocalInvocationId

// CHECK: %SubgroupSize = OpVariable %_ptr_Input_uint Input
// CHECK-NEXT: %SubgroupLocalInvocationId = OpVariable %_ptr_Input_uint Input

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
// CHECK: OpLoad %uint %SubgroupSize
// CHECK: OpLoad %uint %SubgroupSize
// CHECK: OpLoad %uint %SubgroupLocalInvocationId
// CHECK: OpLoad %uint %SubgroupLocalInvocationId
    values[id.x] = WaveGetLaneCount() + WaveGetLaneCount() + WaveGetLaneIndex() + WaveGetLaneIndex();
}
