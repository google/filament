// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
// CHECK: OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK: %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input

[numthreads(8, 8, 8)]
void main(uint3 tid : SV_DispatchThreadId) {}
