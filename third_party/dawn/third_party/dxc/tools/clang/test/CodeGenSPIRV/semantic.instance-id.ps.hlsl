// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Fragment %main "main"
// CHECK-SAME: %in_var_SV_InstanceID

// CHECK:      OpDecorate %in_var_SV_InstanceID Location 0

// CHECK:      %in_var_SV_InstanceID = OpVariable %_ptr_Input_int Input

float4 main(int input: SV_InstanceID) : SV_Target {
    return input;
}
