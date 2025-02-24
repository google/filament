// RUN: %dxc -T ps_6_0 -E main -fspv-target-env=vulkan1.3 -fcgl %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Fragment
// CHECK-SAME: %gl_HelperInvocation

// CHECK:      OpDecorate %gl_HelperInvocation BuiltIn HelperInvocation

// CHECK:      %gl_HelperInvocation = OpVariable %_ptr_Input_bool Input

float4 main() : SV_Target {
// CHECK:      [[val:%[0-9]+]] = OpLoad %bool %gl_HelperInvocation
// CHECK:      OpBranchConditional [[val]]
    float ret = 1.0;

    if (IsHelperLane()) ret = 2.0;

    return ret;
}
