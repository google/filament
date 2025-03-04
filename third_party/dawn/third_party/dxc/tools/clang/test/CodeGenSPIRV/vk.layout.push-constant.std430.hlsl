// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_v2float_uint_3 ArrayStride 8
// CHECK: OpDecorate %_arr_mat3v2float_uint_2 ArrayStride 32
// CHECK: OpDecorate %_arr_v2int_uint_3 ArrayStride 8
// CHECK: OpDecorate %_arr__arr_v2int_uint_3_uint_2 ArrayStride 24

// CHECK: OpMemberDecorate %T 0 Offset 0
// CHECK: OpMemberDecorate %T 1 Offset 32
// CHECK: OpMemberDecorate %T 1 MatrixStride 16
// CHECK: OpMemberDecorate %T 1 RowMajor
// CHECK: OpMemberDecorate %T 2 Offset 96
// CHECK: OpMemberDecorate %T 3 Offset 144
// CHECK: OpMemberDecorate %T 3 MatrixStride 8
// CHECK: OpMemberDecorate %T 3 ColMajor
struct T {
                 float2   f1[3];
    column_major float3x2 f2[2];
    row_major    int3x2   f4[2];
    row_major    float3x2 f3[2];
};

// CHECK: OpDecorate %_arr_v3int_uint_2 ArrayStride 16
// CHECK: OpMemberDecorate %type_PushConstant_S 0 Offset 0
// CHECK: OpMemberDecorate %type_PushConstant_S 1 Offset 16
// CHECK: OpMemberDecorate %type_PushConstant_S 2 Offset 32
// CHECK: OpMemberDecorate %type_PushConstant_S 3 Offset 224
// CHECK: OpMemberDecorate %type_PushConstant_S 4 Offset 256
// CHECK: OpMemberDecorate %type_PushConstant_S 4 MatrixStride 16
// CHECK: OpMemberDecorate %type_PushConstant_S 4 ColMajor

// CHECK: OpDecorate %type_PushConstant_S Block
struct S {
              float    f1;
              float3   f2;
              T        f4;
    row_major int2x3   f5;
    row_major float2x3 f3;
};

[[vk::push_constant]]
S pcs;

float main() : A {
    return pcs.f1;
}
