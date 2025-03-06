// RUN: %dxc -T ps_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

// Deep nested array of matrices
// Depp nested majorness
struct R {                         // Alignment    Offset  Size                              Next
    row_major    float2x3 rf1[3];  // 16(vec4)  -> 0     + 3(array) * stride(2 * 16(vec4)) = 96
    column_major float2x3 rf2[4];  // 8(vec2)   -> 96    + 4(array) * stride(3 * 8(vec2))  = 192
                 float2x3 rf3[2];  // 8(vec2)   -> 192   + 2(array) * stride(3 * 8(vec2))  = 240
    row_major    int2x3   rf4[3];  // 16(vec4)  -> 240   + 3(array) * stride(2 * 16(vec4)) = 336
                 int      rf5;     // 4         -> 336   + 4                               = 340
};                                 // 16(max)                                                352 (340 round up to R alignment)

// Array of scalars, vectors, matrices, and structs
struct S {                         // Alignment   Offset  Size                              Next
    float3       sf1[3];           // 16(vec4) -> 0     + 3(array) * 16(vec4)             = 48
    float        sf2[3];           // 4        -> 48    + 3(array) * 4                    = 60
    R            sf3[4];           // 16       -> 64    + 4(array) * stride(256)          = 1472
    row_major    float3x2 sf4[2];  // 8(vec2)  -> 1472  + 2(array) * stride(3 * 8(vec2))  = 1520
    column_major float3x2 sf5[3];  // 16(vec4) -> 1520  + 3(array) * stride(2 * 16(vec4)) = 1616
                 float3x2 sf6[4];  // 16(vec4) -> 1616  + 4(array) * stride(2 * 16(vec4)) = 1744
    row_major    int3x2   sf7[2];  // 8(vec2)  -> 1744  + 2(array) * stride(3 * 8(vec2))  = 1792
                 float    sf8;     // 4        -> 1792  + 4                               = 1796
};                                 // 16(max)                                               1808 (1796 round up to S alignment)

struct T {        // Alignment    Offset  Size              Next
    R    tf1[2];  // 16        -> 0     + 2(array) * 352  = 704
    S    tf2[3];  // 16        -> 704   + 3(array) * 1808 = 6128
    uint tf3;     // 4         -> 6128  + 4               = 6132
};                // 16(max)                                6144 (6132 round up to T alignment)

struct SBuffer {  // Alignment   Offset   Size                 Next
    T    t[2];       // 16       -> 0      + 2(array) * 6144 = 12288
    bool z;          // 4        -> 12288
};

RWStructuredBuffer<SBuffer> MySBuffer;

// CHECK:      OpDecorate %_arr_mat2v3float_uint_3 ArrayStride 32
// CHECK:      OpDecorate %_arr_mat2v3float_uint_4 ArrayStride 24
// CHECK:      OpDecorate %_arr_mat2v3float_uint_2 ArrayStride 24
// CHECK:      OpDecorate %_arr_v3int_uint_2 ArrayStride 16
// CHECK:      OpDecorate %_arr__arr_v3int_uint_2_uint_3 ArrayStride 32

// CHECK:      OpMemberDecorate %R 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %R 0 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %R 0 ColMajor
// CHECK-NEXT: OpMemberDecorate %R 1 Offset 96
// CHECK-NEXT: OpMemberDecorate %R 1 MatrixStride 8
// CHECK-NEXT: OpMemberDecorate %R 1 RowMajor
// CHECK-NEXT: OpMemberDecorate %R 2 Offset 192
// CHECK-NEXT: OpMemberDecorate %R 2 MatrixStride 8
// CHECK-NEXT: OpMemberDecorate %R 2 RowMajor
// CHECK-NEXT: OpMemberDecorate %R 3 Offset 240
// CHECK-NEXT: OpMemberDecorate %R 4 Offset 336

// CHECK:      OpDecorate %_arr_R_uint_2 ArrayStride 352
// CHECK:      OpDecorate %_arr_v3float_uint_3 ArrayStride 16
// CHECK:      OpDecorate %_arr_float_uint_3 ArrayStride 4
// CHECK:      OpDecorate %_arr_R_uint_4 ArrayStride 352

// CHECK:      OpDecorate %_arr_mat3v2float_uint_2 ArrayStride 24
// CHECK:      OpDecorate %_arr_mat3v2float_uint_3 ArrayStride 32
// CHECK:      OpDecorate %_arr_mat3v2float_uint_4 ArrayStride 32
// CHECK:      OpDecorate %_arr_v2int_uint_3 ArrayStride 8
// CHECK:      OpDecorate %_arr__arr_v2int_uint_3_uint_2 ArrayStride 24

// CHECK:      OpMemberDecorate %S 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %S 1 Offset 48
// CHECK-NEXT: OpMemberDecorate %S 2 Offset 64
// CHECK-NEXT: OpMemberDecorate %S 3 Offset 1472
// CHECK-NEXT: OpMemberDecorate %S 3 MatrixStride 8
// CHECK-NEXT: OpMemberDecorate %S 3 ColMajor
// CHECK-NEXT: OpMemberDecorate %S 4 Offset 1520
// CHECK-NEXT: OpMemberDecorate %S 4 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %S 4 RowMajor
// CHECK-NEXT: OpMemberDecorate %S 5 Offset 1616
// CHECK-NEXT: OpMemberDecorate %S 5 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %S 5 RowMajor
// CHECK-NEXT: OpMemberDecorate %S 6 Offset 1744
// CHECK-NEXT: OpMemberDecorate %S 7 Offset 1792

// CHECK:      OpDecorate %_arr_S_uint_3 ArrayStride 1808

// CHECK:      OpMemberDecorate %T 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %T 1 Offset 704
// CHECK-NEXT: OpMemberDecorate %T 2 Offset 6128

// CHECK:      OpDecorate %_arr_T_uint_2 ArrayStride 6144

// CHECK-NEXT: OpMemberDecorate %SBuffer 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %SBuffer 1 Offset 12288

// CHECK:      OpDecorate %_runtimearr_SBuffer ArrayStride 12304

// CHECK:      OpMemberDecorate %type_RWStructuredBuffer_SBuffer 0 Offset 0
// CHECK-NEXT: OpDecorate %type_RWStructuredBuffer_SBuffer BufferBlock

float4 main() : SV_Target {
    return 1.0;
}
