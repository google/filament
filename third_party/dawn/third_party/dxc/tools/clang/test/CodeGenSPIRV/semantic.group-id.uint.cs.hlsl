// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %main "main" %gl_WorkGroupID
// CHECK: OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
// CHECK: %gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input

// CHECK: [[gl_WorkGroupID:%[0-9]+]] = OpLoad %v3uint %gl_WorkGroupID
// CHECK:   [[uint_GroupID:%[0-9]+]] = OpCompositeExtract %uint [[gl_WorkGroupID]] 0
// CHECK:                           OpStore %param_var_tid [[uint_GroupID]]

RWBuffer<uint> MyBuffer;

[numthreads(8, 8, 8)]
void main(uint tid : SV_GroupID) {
    MyBuffer[0] = tid;
}
