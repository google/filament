// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
// CHECK: OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK: %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3int Input

// CHECK:                  %param_var_tid = OpVariable %_ptr_Function_v2int Function
// CHECK:  [[gl_GlobalInvocationID:%[0-9]+]] = OpLoad %v3int %gl_GlobalInvocationID
// CHECK:  [[int2_DispatchThreadID:%[0-9]+]] = OpVectorShuffle %v2int [[gl_GlobalInvocationID]] [[gl_GlobalInvocationID]] 0 1
// CHECK:                                   OpStore %param_var_tid [[int2_DispatchThreadID]]

RWBuffer<int2> MyBuffer;

[numthreads(1, 1, 1)]
void main(int2 tid : SV_DispatchThreadId) {
    MyBuffer[0] = tid;
}
