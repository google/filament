// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct T1 {
    float2 a;
    float3 b: BBB;
};

struct T2 {
    float2 a;
    float3 b;
};

struct S {
    float  h;
    float1 i : III;
    T1     j;
    float4 k : JJJ;
    T2     l : LLL;
};

// CHECK:    %in_var_X = OpVariable %_ptr_Input_float Input
// CHECK:   %in_var_X1 = OpVariable %_ptr_Input_float Input
// CHECK:   %in_var_X2 = OpVariable %_ptr_Input_v2float Input
// CHECK:   %in_var_X3 = OpVariable %_ptr_Input_v3float Input
// CHECK:   %in_var_X4 = OpVariable %_ptr_Input_v4float Input
// CHECK:   %in_var_X5 = OpVariable %_ptr_Input_v2float Input
// CHECK:   %in_var_X6 = OpVariable %_ptr_Input_v3float Input

// CHECK:  %in_var_Y10 = OpVariable %_ptr_Input_float Input
// CHECK:  %in_var_Y11 = OpVariable %_ptr_Input_float Input
// CHECK:  %in_var_Y12 = OpVariable %_ptr_Input_v2float Input
// CHECK:  %in_var_Y13 = OpVariable %_ptr_Input_v3float Input
// CHECK:  %in_var_Y14 = OpVariable %_ptr_Input_v4float Input
// CHECK:  %in_var_Y15 = OpVariable %_ptr_Input_v2float Input
// CHECK:  %in_var_Y16 = OpVariable %_ptr_Input_v3float Input

// CHECK: %out_var_W20 = OpVariable %_ptr_Output_float Output
// CHECK: %out_var_W21 = OpVariable %_ptr_Output_float Output
// CHECK: %out_var_W22 = OpVariable %_ptr_Output_v2float Output
// CHECK: %out_var_W23 = OpVariable %_ptr_Output_v3float Output
// CHECK: %out_var_W24 = OpVariable %_ptr_Output_v4float Output
// CHECK: %out_var_W25 = OpVariable %_ptr_Output_v2float Output
// CHECK: %out_var_W26 = OpVariable %_ptr_Output_v3float Output

// CHECK:   %out_var_Z = OpVariable %_ptr_Output_float Output
// CHECK:  %out_var_Z1 = OpVariable %_ptr_Output_float Output
// CHECK:  %out_var_Z2 = OpVariable %_ptr_Output_v2float Output
// CHECK:  %out_var_Z3 = OpVariable %_ptr_Output_v3float Output
// CHECK:  %out_var_Z4 = OpVariable %_ptr_Output_v4float Output
// CHECK:  %out_var_Z5 = OpVariable %_ptr_Output_v2float Output
// CHECK:  %out_var_Z6 = OpVariable %_ptr_Output_v3float Output

S main(S input1: X, S input2: Y10, out S output1: Z) : W20 {
    output1 = input1;
    return input2;
}
