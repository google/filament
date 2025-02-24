// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_LocalInvocationID
// CHECK: OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
// CHECK: %gl_LocalInvocationID = OpVariable %_ptr_Input_v3int Input

// CHECK:               %param_var_gtid = OpVariable %_ptr_Function_v2int Function
// CHECK: [[gl_LocalInvocationID:%[0-9]+]] = OpLoad %v3int %gl_LocalInvocationID
// CHECK:   [[int2_GroupThreadID:%[0-9]+]] = OpVectorShuffle %v2int [[gl_LocalInvocationID]] [[gl_LocalInvocationID]] 0 1
// CHECK:                                 OpStore %param_var_gtid [[int2_GroupThreadID]]

RWBuffer<int2> MyBuffer;

[numthreads(1, 1, 1)]
void main(int2 gtid : SV_GroupThreadID) {
    MyBuffer[0] = gtid;
}
