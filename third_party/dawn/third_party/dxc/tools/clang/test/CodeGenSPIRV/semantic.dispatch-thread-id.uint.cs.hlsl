// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
// CHECK: OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK: %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input

// CHECK: [[gl_GlobalInvocationID:%[0-9]+]] = OpLoad %v3uint %gl_GlobalInvocationID
// CHECK: [[uint_DispatchThreadID:%[0-9]+]] = OpCompositeExtract %uint [[gl_GlobalInvocationID]] 0
// CHECK:                                  OpStore %param_var_tid [[uint_DispatchThreadID]]

RWBuffer<uint> MyBuffer;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadId) {
    MyBuffer[0] = tid;
}
