// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_float_uint_2 ArrayStride 4
// CHECK: OpDecorate %_arr_v3float_uint_2 ArrayStride 16
// CHECK: OpDecorate %_arr_mat2v3float_uint_2 ArrayStride 32
// CHECK: OpDecorate %_arr_mat2v3float_uint_2_0 ArrayStride 24
// CHECK: OpDecorate %_arr_v3int_uint_2 ArrayStride 16
// CHECK: OpDecorate %_arr__arr_v3int_uint_2_uint_2 ArrayStride 32

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 16
// CHECK: OpMemberDecorate %S 2 Offset 48
// CHECK: OpMemberDecorate %S 2 MatrixStride 16
// CHECK: OpMemberDecorate %S 2 ColMajor
// CHECK: OpMemberDecorate %S 3 Offset 112
// CHECK: OpMemberDecorate %S 3 MatrixStride 8
// CHECK: OpMemberDecorate %S 3 RowMajor
// CHECK: OpMemberDecorate %S 4 Offset 160
// CHECK: OpMemberDecorate %S 4 MatrixStride 8
// CHECK: OpMemberDecorate %S 4 RowMajor
// CHECK: OpMemberDecorate %S 5 Offset 208
// CHECK: OpMemberDecorate %S 6 Offset 272

// CHECK: OpDecorate %_arr_S_uint_2 ArrayStride 288

// CHECK: OpMemberDecorate %type_myTbuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_myTbuffer 0 NonWritable
// CHECK: OpMemberDecorate %type_myTbuffer 1 Offset 576
// CHECK: OpMemberDecorate %type_myTbuffer 1 NonWritable

// CHECK: OpDecorate %type_myTbuffer BufferBlock

struct S {
                 float    a[2];
                 float3   b[2];
    row_major    float2x3 c[2];
    column_major float2x3 d[2];
                 float2x3 e[2];
    row_major    int2x3   f[2];
                 int      g;
};

tbuffer myTbuffer : register(t0)
{
  S s[2];
  uint t;
};

float main() : A {
  return 1.0;
}
