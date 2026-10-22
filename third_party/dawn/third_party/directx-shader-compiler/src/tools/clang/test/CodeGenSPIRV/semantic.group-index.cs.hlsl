// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_LocalInvocationIndex
// CHECK: OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex
// CHECK: %gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input

[numthreads(1,1,1)]
void main(uint gid : SV_GroupIndex) : A {}
