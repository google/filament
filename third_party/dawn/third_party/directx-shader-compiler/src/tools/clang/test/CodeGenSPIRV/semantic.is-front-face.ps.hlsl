// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint Fragment %main "main" %gl_FrontFacing %out_var_SV_Target

// CHECK: OpDecorate %gl_FrontFacing BuiltIn FrontFacing

// CHECK: %gl_FrontFacing = OpVariable %_ptr_Input_bool Input

float4 main(bool ff: SV_IsFrontFace) : SV_Target {
    return ff;
}
