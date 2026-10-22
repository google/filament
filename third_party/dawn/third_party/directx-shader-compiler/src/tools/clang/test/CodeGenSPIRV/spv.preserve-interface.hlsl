// RUN: %dxc -T ps_6_0 -E main -O3 -fspv-preserve-interface  %s -spirv | FileCheck %s

float4 main(float2 a : A, /* unused */ float3 b : B, float2 c : C) : SV_Target {
  return float4(a, c);
}

// CHECK: OpName %in_var_A "in.var.A"
// CHECK: OpName %in_var_B "in.var.B"
// CHECK: OpName %in_var_C "in.var.C"
// CHECK: OpDecorate %in_var_A Location 0
// CHECK: OpDecorate %in_var_B Location 1
// CHECK: OpDecorate %in_var_C Location 2
// CHECK: %in_var_A = OpVariable %_ptr_Input_v2float Input
// CHECK: %in_var_B = OpVariable %_ptr_Input_v3float Input
// CHECK: %in_var_C = OpVariable %_ptr_Input_v2float Input
