// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:       [[sv_pos:%[0-9]+]] = OpLoad %v4float %in_var_SV_Position
// CHECK-NEXT:                       OpStore %param_var_pos [[sv_pos]]
// CHECK-NEXT:                       OpFunctionCall %void %src_main %param_var_pos
// CHECK-NEXT: [[var_pos:%[0-9]+]] = OpLoad %v4float %param_var_pos
// CHECK-NEXT:                       OpStore %gl_Position [[var_pos]]
void main(inout float4 pos : SV_Position) {}
