// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main (
// CHECK:  %in_var_A = OpVariable %_ptr_Input_v3float Input
    in  float3 used_in_var    : A,
// CHECK:  %in_var_B = OpVariable %_ptr_Input_v3float Input
    in  float3 unused_in_var  : B,
// CHECK: %out_var_C = OpVariable %_ptr_Output_v3float Output
    out float3 used_out_var   : C,
// CHECK: %out_var_D = OpVariable %_ptr_Output_v3float Output
    out float3 unused_out_var : D
) {
// CHECK:      [[a:%[0-9]+]] = OpLoad %v3float %in_var_A
// CHECK-NEXT:              OpStore %param_var_used_in_var [[a]]
// No writing to %param_var_unused_in_var
// CHECK-NEXT:   {{%[0-9]+}} = OpLoad %v3float %in_var_B
// CHECK-NEXT:   {{%[0-9]+}} = OpFunctionCall %void %src_main %param_var_used_in_var %param_var_unused_in_var %param_var_used_out_var %param_var_unused_out_var
// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %param_var_used_out_var
// CHECK-NEXT:              OpStore %out_var_C [[c]]
// No writing to %out_var_D
// CHECK-NEXT:              OpReturn
    used_out_var = used_in_var;
}
