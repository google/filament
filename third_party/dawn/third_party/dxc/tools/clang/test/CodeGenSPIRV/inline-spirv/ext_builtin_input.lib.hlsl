// RUN: %dxc -spirv -enable-16bit-types -T lib_6_7 -HV 202x %s | FileCheck %s

// CHECK: OpEntryPoint Fragment %psmain "psmain" %gl_HelperInvocation %out_var_SV_Target0
// CHECK: OpDecorate %gl_HelperInvocation BuiltIn HelperInvocation
// CHECK: %gl_HelperInvocation = OpVariable %_ptr_Input_bool Input

[[vk::ext_builtin_input(23 /* BuiltInHelperInvocation */)]]
static const bool HelperInvocation;

[shader("pixel")]
float4 psmain() : SV_Target0 {

    if (HelperInvocation)
      return 0;
    return 1;
}
