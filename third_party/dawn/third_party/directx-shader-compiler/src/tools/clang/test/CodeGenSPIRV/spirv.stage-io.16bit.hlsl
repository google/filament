// RUN: %dxc -T vs_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability StorageInputOutput16

// CHECK: OpExtension "SPV_KHR_16bit_storage"

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

// CHECK: %half = OpTypeFloat 16
// CHECK-NOT: %float = OpTypeFloat 32

// CHECK:  %in_var_A = OpVariable %_ptr_Input__arr_v2half_uint_4 Input
// CHECK:  %in_var_B = OpVariable %_ptr_Input__arr_v3ushort_uint_2 Input
// CHECK:  %in_var_C = OpVariable %_ptr_Input_short Input
// CHECK:  %in_var_D = OpVariable %_ptr_Input_v2ushort Input
// CHECK:  %in_var_E = OpVariable %_ptr_Input_mat3v2half Input

// CHECK: %out_var_A = OpVariable %_ptr_Output_mat2v3half Output
// CHECK: %out_var_B = OpVariable %_ptr_Output__arr_v2short_uint_4 Output
// CHECK: %out_var_C = OpVariable %_ptr_Output_half Output
// CHECK: %out_var_D = OpVariable %_ptr_Output_v2short Output
// CHECK: %out_var_E = OpVariable %_ptr_Output_v3ushort Output

struct VSOut {
    half2x3   outA    : A; // 2 locations: 0, 1
    int16_t2  outB[4] : B; // 4 locations: 2, 3, 4, 5
    half      outC    : C; // 1 location : 6
    int16_t2  outD    : D; // 1 location : 7
    uint16_t3 outE    : E; // 1 location : 8
};

VSOut main(
    half2        inA[4] : A, // 4 locations: 0, 1, 2, 3
    uint16_t2x3  inB    : B, // 2 locations: 4, 5
    int16_t      inC    : C, // 1 location : 6
    uint16_t2    inD    : D, // 1 location : 7
    float16_t3x2 inE    : E  // 3 location : 8, 9, 10
) {
    VSOut o;
    o.outA    = inA[0].x;
    o.outB[0] = inB[0][0];
    o.outC    = inC.x;
    o.outD    = inD[0];
    o.outE    = inE[0][0];
    return o;
}
