// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

struct R {     // Alignment                           Offset     Size       Next
    float2 rf; // 8(vec2)                          -> 0        + 8(vec2)  = 8
};             // 16(8 round up to vec4 alignment)               8          16(8 round up to R max alignment)

struct S {      // Alignment    Offset                                Size        Next
    R      sf1; // 16        -> 0                                   + 16        = 16
    float  sf2; // 4         -> 16                                  + 4         = 20
    float3 sf3; // 16(vec4)  -> 32 (20 round up to vec4 alignment)  + 12(vec3)  = 44
    float  sf4; // 4         -> 44                                  + 4         = 48
};              // 16(max)                                                        48(48 round up to S max alignment)

struct T {                     // Alignment     Offset                               Size              = Next
              int      tf1;    // 4          -> 0                                  + 4                 = 4
              R        tf2[3]; // 16         -> 16 (4 rounded up to R alignment)   + 3 * stride(16)    = 64
              float3x2 tf3;    // 16(vec4)   -> 64 (64 round up to vec4 alignment) + 2 * stride(vec4)  = 96
              S        tf4;    // 16         -> 96 (96 round up to S alignment)    + 48                = 144
              float    tf5;    // 4          -> 144                                + 4                 = 148
    row_major int3x2   tf6;    // 16(vec4)   -> 160 (148 rounded up to vec4)       + 3 * stride(vec4)  = 208
};                             // 16(max)                                                                208(208 round up to T max alignment)

cbuffer MyCBuffer {              // Alignment   Offset                                 Size                     Next
                 bool     a;     // 4        -> 0                                    +     4                  = 4
                 uint1    b;     // 4        -> 4                                    +     4                  = 8
                 float3   c;     // 16(vec4) -> 16 (8 round up to vec4 alignment)    + 3 * 4                  = 28
    row_major    float2x3 d;     // 16(vec4) -> 32 (28 round up to vec4 alignment)   + 2 * stride(vec4)       = 64
    column_major float2x3 e;     // 16(vec4) -> 64 (64 round up to vec4 alignment)   + 3 * stride(vec4)       = 112
                 float2x1 f;     // 8(vec2)  -> 112 (112 round up to vec2 aligment)  + 2 * 4                  = 120
    row_major    float2x3 g[3];  // 16(vec4) -> 128 (120 round up to vec4 alignment) + 3 * 2 * stride(vec4)   = 224
    column_major float2x2 h[4];  // 16(vec4) -> 224 (224 round up to vec4 alignment) + 4 * 2 * stride(vec4)   = 352
                 T        t;     // 16       -> 352 (352 round up to vec4 alignment) + 208                    = 560
    row_major    int2x3   y;     // 16(vec4) -> 560 (560 round up to vec4 alignment) + 2 * stride(vec4)       = 592
                 float    z;     // 4        -> 592
};

// CHECK:      OpDecorate %_arr_mat2v3float_uint_3 ArrayStride 32
// CHECK:      OpDecorate %_arr_mat2v2float_uint_4 ArrayStride 32

// CHECK:      OpMemberDecorate %R 0 Offset 0

// CHECK:      OpDecorate %_arr_R_uint_3 ArrayStride 16

// CHECK:      OpMemberDecorate %S 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %S 1 Offset 16
// CHECK-NEXT: OpMemberDecorate %S 2 Offset 32
// CHECK-NEXT: OpMemberDecorate %S 3 Offset 44

// CHECK:      OpMemberDecorate %T 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %T 1 Offset 16
// CHECK-NEXT: OpMemberDecorate %T 2 Offset 64
// CHECK-NEXT: OpMemberDecorate %T 2 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %T 2 RowMajor
// CHECK-NEXT: OpMemberDecorate %T 3 Offset 96
// CHECK-NEXT: OpMemberDecorate %T 4 Offset 144
// CHECK-NEXT: OpMemberDecorate %T 5 Offset 160

// CHECK:      OpMemberDecorate %type_MyCBuffer 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 1 Offset 4
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 2 Offset 16
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 3 Offset 32
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 3 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 3 ColMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 4 Offset 64
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 4 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 4 RowMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 5 Offset 112
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 6 Offset 128
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 6 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 6 ColMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 7 Offset 224
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 7 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 7 RowMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 8 Offset 352
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 9 Offset 560
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 10 Offset 592
// CHECK-NEXT: OpDecorate %type_MyCBuffer Block

float main() : A {
    return 1.0;
}
