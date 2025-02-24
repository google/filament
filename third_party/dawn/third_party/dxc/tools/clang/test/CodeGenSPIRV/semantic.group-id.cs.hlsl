// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_WorkGroupID
// CHECK: OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
// CHECK: %gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input

[numthreads(8, 8, 8)]
void main(uint3 tid : SV_GroupID) {}
