// RUN: %dxc -T vs_6_0 -E main -Zpr -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_mat2v3float_uint_5 ArrayStride 32
// CHECK: OpDecorate %_arr_mat2v3float_uint_5_0 ArrayStride 48

// CHECK: OpDecorate %_arr_v3int_uint_2 ArrayStride 16
// CHECK: OpDecorate %_arr__arr_v3int_uint_2_uint_5 ArrayStride 32

// CHECK: OpMemberDecorate %type_MyCBuffer 0 ColMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 1 RowMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 2 ColMajor

// CHECK: %type_MyCBuffer = OpTypeStruct %_arr_mat2v3float_uint_5 %_arr_mat2v3float_uint_5_0 %_arr_mat2v3float_uint_5
cbuffer MyCBuffer {
    row_major    float2x3 matrices1[5];
    column_major float2x3 matrices2[5];
                 float2x3 matrices3[5];

    row_major    int2x3   matrices4[5];
                 int2x3   matrices5[5];
}

void main() {
    // Check that the result types for access chains are correct
// CHECK: {{%[0-9]+}} = OpAccessChain %_ptr_Uniform__arr_mat2v3float_uint_5 %MyCBuffer %int_0
// CHECK: {{%[0-9]+}} = OpAccessChain %_ptr_Uniform__arr_mat2v3float_uint_5_0 %MyCBuffer %int_1
// CHECK: {{%[0-9]+}} = OpAccessChain %_ptr_Uniform__arr_mat2v3float_uint_5 %MyCBuffer %int_2
    float2x3 m1 = matrices1[1];
    float2x3 m2 = matrices2[2];
    float2x3 m3 = matrices3[3];

    // Note: Since non-fp matrices are represented as arrays of vectors, and
    // due to layout decoration on the rhs of the assignments below,
    // a load and store is performed for each vector.

// CHECK:          [[ptr_matrices4:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr__arr_v3int_uint_2_uint_5 %MyCBuffer %int_3
// CHECK-NEXT:   [[ptr_matrices4_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_v3int_uint_2 [[ptr_matrices4]] %int_1
// CHECK-NEXT:       [[matrices4_1:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 [[ptr_matrices4_1]]
// CHECK-NEXT:  [[matrices4_1_row0:%[0-9]+]] = OpCompositeExtract %v3int [[matrices4_1]] 0
// CHECK-NEXT:  [[matrices4_1_row1:%[0-9]+]] = OpCompositeExtract %v3int [[matrices4_1]] 1
// CHECK-NEXT:               [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2_0 [[matrices4_1_row0]] [[matrices4_1_row1]]
// CHECK-NEXT:                              OpStore %m4 [[tmp]]
    int2x3 m4 = matrices4[1];
// CHECK:          [[ptr_matrices5:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr__arr_v3int_uint_2_uint_5 %MyCBuffer %int_4
// CHECK-NEXT:   [[ptr_matrices5_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_v3int_uint_2 [[ptr_matrices5]] %int_2
// CHECK-NEXT:       [[matrices5_2:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 [[ptr_matrices5_2]]
// CHECK-NEXT: [[matrices_5_2_row0:%[0-9]+]] = OpCompositeExtract %v3int [[matrices5_2]] 0
// CHECK-NEXT: [[matrices_5_2_row1:%[0-9]+]] = OpCompositeExtract %v3int [[matrices5_2]] 1
// CHECK-NEXT:               [[tmp_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2_0 [[matrices_5_2_row0]] [[matrices_5_2_row1]]
// CHECK-NEXT:                              OpStore %m5 [[tmp_0]]
    int2x3 m5 = matrices5[2];
}
