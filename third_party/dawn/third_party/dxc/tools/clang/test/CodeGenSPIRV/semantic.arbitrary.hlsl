// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint Vertex %main "main" %in_var_AAA %in_var_B %in_var_CC %out_var_DDDD

// CHECK: OpDecorate %in_var_AAA Location 0
// CHECK: OpDecorate %in_var_B Location 1
// CHECK: OpDecorate %in_var_CC Location 2
// CHECK: OpDecorate %out_var_DDDD Location 0

// CHECK: %in_var_AAA = OpVariable %_ptr_Input_v4float Input
// CHECK: %in_var_B = OpVariable %_ptr_Input_int Input
// CHECK: %in_var_CC = OpVariable %_ptr_Input_mat2v3float Input
// CHECK: %out_var_DDDD = OpVariable %_ptr_Output_float Output

float main(float4 a: AAA, int b: B, float2x3 c: CC) : DDDD {
    return 1.0;
}
