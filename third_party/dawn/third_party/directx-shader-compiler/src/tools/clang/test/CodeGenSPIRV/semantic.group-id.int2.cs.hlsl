// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_WorkGroupID
// CHECK: OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
// CHECK: %gl_WorkGroupID = OpVariable %_ptr_Input_v3int Input

// CHECK:         %param_var_tid = OpVariable %_ptr_Function_v2int Function
// CHECK: [[gl_WorkGrouID:%[0-9]+]] = OpLoad %v3int %gl_WorkGroupID
// CHECK:  [[int2_GroupID:%[0-9]+]] = OpVectorShuffle %v2int [[gl_WorkGrouID]] [[gl_WorkGrouID]] 0 1
// CHECK:                          OpStore %param_var_tid [[int2_GroupID]]

RWBuffer<int2> MyBuffer;

[numthreads(1, 1, 1)]
void main(int2 tid : SV_GroupID) {
    MyBuffer[0] = tid;
}
