// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint Fragment %main "main" %gl_SampleMask %gl_SampleMask_0

// CHECK: OpDecorate %gl_SampleMask BuiltIn SampleMask
// CHECK: OpDecorate %gl_SampleMask_0 BuiltIn SampleMask

// CHECK: %gl_SampleMask = OpVariable %_ptr_Input__arr_uint_uint_1 Input
// CHECK: %gl_SampleMask_0 = OpVariable %_ptr_Output__arr_uint_uint_1 Output

uint main(uint inCov : SV_Coverage) : SV_Coverage {
    return inCov;
}
