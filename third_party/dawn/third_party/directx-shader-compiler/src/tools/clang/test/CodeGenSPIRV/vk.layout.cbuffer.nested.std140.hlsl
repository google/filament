// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

// Deep nested array of matrices
// Deep nested majorness
struct R {                         // Alignment    Offset  Size                              Next
    row_major    float2x3 rf1[3];  // 16(vec4)  -> 0     + 3(array) * stride(2 * 16(vec4)) = 96
    column_major float2x3 rf2[4];  // 16(vec4)  -> 96    + 4(array) * stride(3 * 16(vec4)) = 288
                 float2x3 rf3[2];  // 16(vec4)  -> 288   + 2(array) * stride(3 * 16(vec4)) = 384
    row_major    int2x3   rf4[2];  // 16(vec4)  -> 384   + 2(array) * stride(2 * 16(vec4)) = 448
                 int      rf5;     // 4         -> 448   + 4                               = 452
};                                 // 16(max)                                                464 (452 round up to R alignment)

// Array of scalars, vectors, matrices, and structs
struct S {                         // Alignment   Offset  Size                              Next
    float3       sf1[3];           // 16(vec4) -> 0     + 3(array) * 16(vec4)             = 48
    float        sf2[3];           // 4        -> 48    + 3(array) * 16(vec4)             = 96
    R            sf3[4];           // 16       -> 96    + 4(array) * stride(464)          = 1952
    row_major    float3x2 sf4[2];  // 16(vec4) -> 1952  + 2(array) * stride(3 * 16(vec4)) = 2048
    column_major float3x2 sf5[3];  // 16(vec4) -> 2048  + 3(array) * stride(2 * 16(vec4)) = 2144
                 float3x2 sf6[4];  // 16(vec4) -> 2144  + 4(array) * stride(2 * 16(vec4)) = 2272
                 float    sf7;     // 4        -> 2272  + 4                               = 2276
};                                 // 16(max)                                               2288 (2276 round up to S alignment)

struct T {        // Alignment    Offset  Size              Next
    R    tf1[2];  // 16        -> 0     + 2(array) * 464  = 928
    S    tf2[3];  // 16        -> 928   + 3(array) * 2288 = 7792
    uint tf3;     // 4         -> 7792  + 4               = 7796
};                // 16(max)                                7808 (7796 round up to T alignment)

cbuffer MyCbuffer {  // Alignment   Offset   Size              Next
    T    t[2];       // 16       -> 0      + 2(array) * 7808 = 15616
    bool z;          // 4        -> 15616
};

// CHECK:      OpDecorate %_arr_mat2v3float_uint_3 ArrayStride 32
// CHECK:      OpDecorate %_arr_mat2v3float_uint_4 ArrayStride 48
// CHECK:      OpDecorate %_arr_mat2v3float_uint_2 ArrayStride 48
// CHECK:      OpDecorate %_arr_v3int_uint_2 ArrayStride 16
// CHECK:      OpDecorate %_arr__arr_v3int_uint_2_uint_2 ArrayStride 32

// CHECK:      OpMemberDecorate %R 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %R 0 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %R 0 ColMajor
// CHECK-NEXT: OpMemberDecorate %R 1 Offset 96
// CHECK-NEXT: OpMemberDecorate %R 1 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %R 1 RowMajor
// CHECK-NEXT: OpMemberDecorate %R 2 Offset 288
// CHECK-NEXT: OpMemberDecorate %R 2 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %R 2 RowMajor
// CHECK-NEXT: OpMemberDecorate %R 3 Offset 384
// CHECK-NEXT: OpMemberDecorate %R 4 Offset 448

// CHECK:      OpDecorate %_arr_R_uint_2 ArrayStride 464
// CHECK:      OpDecorate %_arr_v3float_uint_3 ArrayStride 16
// CHECK:      OpDecorate %_arr_float_uint_3 ArrayStride 16
// CHECK:      OpDecorate %_arr_R_uint_4 ArrayStride 464

// CHECK:      OpDecorate %_arr_mat3v2float_uint_2 ArrayStride 48
// CHECK:      OpDecorate %_arr_mat3v2float_uint_3 ArrayStride 32
// CHECK:      OpDecorate %_arr_mat3v2float_uint_4 ArrayStride 32

// CHECK:      OpMemberDecorate %S 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %S 1 Offset 48
// CHECK-NEXT: OpMemberDecorate %S 2 Offset 96
// CHECK-NEXT: OpMemberDecorate %S 3 Offset 1952
// CHECK-NEXT: OpMemberDecorate %S 3 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %S 3 ColMajor
// CHECK-NEXT: OpMemberDecorate %S 4 Offset 2048
// CHECK-NEXT: OpMemberDecorate %S 4 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %S 4 RowMajor
// CHECK-NEXT: OpMemberDecorate %S 5 Offset 2144
// CHECK-NEXT: OpMemberDecorate %S 5 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %S 5 RowMajor
// CHECK-NEXT: OpMemberDecorate %S 6 Offset 2272

// CHECK-NEXT: OpDecorate %_arr_S_uint_3 ArrayStride 2288

// CHECK:      OpMemberDecorate %T 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %T 1 Offset 928
// CHECK-NEXT: OpMemberDecorate %T 2 Offset 7792

// CHECK:      OpDecorate %_arr_T_uint_2 ArrayStride 7808

// CHECK-NEXT: OpMemberDecorate %type_MyCbuffer 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %type_MyCbuffer 1 Offset 15616

// CHECK:      OpDecorate %type_MyCbuffer Block
float main() : A {
    return 1.0;
}
