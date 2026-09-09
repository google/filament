// RUN: %dxc -T ps_6_0 -E main -fspv-target-env=vulkan1.3 -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Fragment
// CHECK-SAME: %gl_HelperInvocation

// CHECK:      OpDecorate %gl_HelperInvocation BuiltIn HelperInvocation

// CHECK:      %gl_HelperInvocation = OpVariable %_ptr_Input_bool Input

float4 main([[vk::builtin("HelperInvocation")]] bool isHI : HI) : SV_Target {
// CHECK:      [[val:%[0-9]+]] = OpLoad %bool %gl_HelperInvocation
// CHECK-NEXT: OpStore %param_var_isHI [[val]]
    float ret = 1.0;

    if (isHI) ret = 2.0;

    return ret;
}
