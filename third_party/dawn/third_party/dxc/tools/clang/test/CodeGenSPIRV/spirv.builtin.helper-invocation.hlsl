// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK-NOT: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn HelperInvocation

// CHECK: %HelperInvocation = OpVariable %_ptr_Private_bool Private

float4 main([[vk::builtin("HelperInvocation")]] bool isHI : HI) : SV_Target {
// CHECK:      [[val:%[0-9]+]] = OpLoad %bool %HelperInvocation
// CHECK-NEXT: OpStore %param_var_isHI [[val]]
    float ret = 1.0;

    if (isHI) ret = 2.0;

    return ret;
}
// CHECK:      %module_init = OpFunction %void None
// CHECK:      %module_init_bb = OpLabel
// CHECK:      [[HelperInvocation:%[0-9]+]] = OpIsHelperInvocationEXT %bool
// CHECK-NEXT: OpStore %HelperInvocation [[HelperInvocation]]
