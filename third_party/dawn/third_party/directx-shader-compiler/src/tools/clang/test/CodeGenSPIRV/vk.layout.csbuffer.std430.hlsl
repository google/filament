// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

// CHECK-DAG: OpDecorate %_arr_float_uint_2 ArrayStride 4
// CHECK-DAG: OpDecorate %_arr_v3float_uint_2 ArrayStride 16
// CHECK-DAG: OpDecorate %_arr_mat2v3float_uint_2 ArrayStride 32
// CHECK-DAG: OpDecorate %_arr_mat2v3float_uint_2_0 ArrayStride 24
// CHECK-DAG: OpDecorate %_arr_v3int_uint_2 ArrayStride 16
// CHECK-DAG: OpDecorate %_arr__arr_v3int_uint_2_uint_2 ArrayStride 32

// CHECK-DAG: OpMemberDecorate %S 0 Offset 0
// CHECK-DAG: OpMemberDecorate %S 1 Offset 16
// CHECK-DAG: OpMemberDecorate %S 2 Offset 48
// CHECK-DAG: OpMemberDecorate %S 2 MatrixStride 16
// CHECK-DAG: OpMemberDecorate %S 2 ColMajor
// CHECK-DAG: OpMemberDecorate %S 3 Offset 112
// CHECK-DAG: OpMemberDecorate %S 3 MatrixStride 8
// CHECK-DAG: OpMemberDecorate %S 3 RowMajor
// CHECK-DAG: OpMemberDecorate %S 4 Offset 160
// CHECK-DAG: OpMemberDecorate %S 4 MatrixStride 8
// CHECK-DAG: OpMemberDecorate %S 4 RowMajor
// CHECK-DAG: OpMemberDecorate %S 5 Offset 208
// CHECK-DAG: OpMemberDecorate %S 6 Offset 272

// CHECK-DAG: OpDecorate %_arr_S_uint_2 ArrayStride 288

// CHECK-DAG: OpMemberDecorate %T 0 Offset 0
// CHECK-DAG: OpMemberDecorate %T 1 Offset 576

// CHECK-DAG: OpDecorate %_runtimearr_T ArrayStride 592

// CHECK-DAG: OpMemberDecorate %type_ConsumeStructuredBuffer_T 0 Offset 0
// CHECK-DAG: OpDecorate %type_ConsumeStructuredBuffer_T BufferBlock

// CHECK-DAG: OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
// CHECK-DAG: OpDecorate %type_ACSBuffer_counter BufferBlock
struct S {
                 float    a[2];
                 float3   b[2];
    row_major    float2x3 c[2];
    column_major float2x3 d[2];
                 float2x3 e[2];
    row_major    int2x3   f[2];
                 int      g;
};

struct T {
    S    s[2];
    uint t;
};

ConsumeStructuredBuffer<T> buffer2;

float main() : A {
    return 1.0;
}
