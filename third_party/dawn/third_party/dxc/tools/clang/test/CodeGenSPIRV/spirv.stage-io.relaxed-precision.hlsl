// RUN: %dxc -T vs_6_2 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %in_var_A Location 0
// CHECK: OpDecorate %in_var_B Location 4
// CHECK: OpDecorate %in_var_C Location 6
// CHECK: OpDecorate %in_var_D Location 7
// CHECK: OpDecorate %in_var_E Location 8

// CHECK: OpDecorate %out_var_A Location 0
// CHECK: OpDecorate %out_var_B Location 2
// CHECK: OpDecorate %out_var_C Location 6
// CHECK: OpDecorate %out_var_D Location 7
// CHECK: OpDecorate %out_var_E Location 8

// CHECK: OpDecorate %in_var_A RelaxedPrecision
// CHECK: OpDecorate %in_var_B RelaxedPrecision
// CHECK: OpDecorate %in_var_C RelaxedPrecision
// CHECK: OpDecorate %in_var_D RelaxedPrecision
// CHECK: OpDecorate %in_var_E RelaxedPrecision

// CHECK: OpDecorate %out_var_A RelaxedPrecision
// CHECK: OpDecorate %out_var_B RelaxedPrecision
// CHECK: OpDecorate %out_var_C RelaxedPrecision
// CHECK: OpDecorate %out_var_D RelaxedPrecision
// CHECK: OpDecorate %out_var_E RelaxedPrecision

// CHECK: %float = OpTypeFloat 32
// CHECK-NOT: %half = OpTypeFloat 16

// CHECK:  %in_var_A = OpVariable %_ptr_Input__arr_v2float_uint_4 Input
// CHECK:  %in_var_B = OpVariable %_ptr_Input__arr_v3uint_uint_2 Input
// CHECK:  %in_var_C = OpVariable %_ptr_Input_int Input
// CHECK:  %in_var_D = OpVariable %_ptr_Input_v2uint Input
// CHECK:  %in_var_E = OpVariable %_ptr_Input_mat3v2float Input

// CHECK: %out_var_A = OpVariable %_ptr_Output_mat2v3float Output
// CHECK: %out_var_B = OpVariable %_ptr_Output__arr_v2int_uint_4 Output
// CHECK: %out_var_C = OpVariable %_ptr_Output_float Output
// CHECK: %out_var_D = OpVariable %_ptr_Output_v2int Output
// CHECK: %out_var_E = OpVariable %_ptr_Output_v3uint Output

struct VSOut {
    min16float2x3   outA    : A; // 2 locations: 0, 1
    min16int2       outB[4] : B; // 4 locations: 2, 3, 4, 5
    min16float      outC    : C; // 1 location : 6
    min16int2       outD    : D; // 1 location : 7
    min16uint3      outE    : E; // 1 location : 8
};

VSOut main(
    min16float2        inA[4] : A, // 4 locations: 0, 1, 2, 3
    min16uint2x3       inB    : B, // 2 locations: 4, 5
    min16int           inC    : C, // 1 location : 6
    min16uint2         inD    : D, // 1 location : 7
    min16float3x2      inE    : E  // 3 location : 8, 9, 10
) {
    VSOut o;
    o.outA    = inA[0].x;
    o.outB[0] = inB[0][0];
    o.outC    = inC.x;
    o.outD    = inD[0];
    o.outE    = inE[0][0];
    return o;
}
