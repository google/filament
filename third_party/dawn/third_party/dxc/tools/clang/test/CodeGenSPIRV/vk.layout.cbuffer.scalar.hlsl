// RUN: %dxc -T vs_6_2 -E main -fvk-use-scalar-layout -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

struct R {     // Alignment       Offset     Size       Next
    double rf; // 8            -> 0        + 8        = 8
};             // 8                                     8

struct S {      // Alignment    Offset                                Size        Next
    R      sf1; // 8         -> 0                                   + 8         = 8
    float  sf2; // 4         -> 8                                   + 4         = 12
    float3 sf3; // 4         -> 12                                  + 4 * 3     = 24
    float  sf4; // 4         -> 24                                  + 4         = 28
};              // 8(max)                                                         28

struct T {                     // Alignment     Offset                               Size              = Next
              int      tf1;    // 4          -> 0                                  + 4                 = 4
              R        tf2[3]; // 8          -> 8 (4 round up to R alignment)      + 3 * stride(8)     = 32
              float3x2 tf3;    // 4          -> 32                                 + 4 * 3 * 2         = 56
              S        tf4;    // 8          -> 56                                 + 28                = 84
             float16_t tf5;    // 2          -> 84                                 + 2                 = 86
              float    tf6;    // 4          -> 88 (86 round up to float align)    + 4                 = 92
};                             // 8(max)                                                                 92

cbuffer MyCBuffer {              // Alignment   Offset                                 Size                     Next
                 bool     a;     // 4        -> 0                                    +     4                  = 4
                 uint1    b;     // 4        -> 4                                    +     4                  = 8
                 float3   c;     // 4        -> 8                                    + 3 * 4                  = 20
    row_major    float2x3 d;     // 4        -> 20                                   + 4 * 2 * 3              = 44
    column_major float2x3 e;     // 4        -> 44                                   + 4 * 2 * 3              = 68
                 float2x1 f;     // 4        -> 68                                   + 4 * 2                  = 76
    row_major    float2x3 g[3];  // 4        -> 76                                   + 4 * 2 * 3 * 3          = 148
    column_major float2x2 h[4];  // 4        -> 148                                  + 4 * 2 * 2 * 4          = 212
                 T        t;     // 8        -> 216 (212 round up to T    alignment) + 92                     = 308
                 float    z;     // 4        -> 308
};

// CHECK:      OpDecorate %_arr_mat2v3float_uint_3 ArrayStride 24
// CHECK:      OpDecorate %_arr_mat2v2float_uint_4 ArrayStride 16

// CHECK:      OpMemberDecorate %R 0 Offset 0

// CHECK:      OpDecorate %_arr_R_uint_3 ArrayStride 8

// CHECK:      OpMemberDecorate %S 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %S 1 Offset 8
// CHECK-NEXT: OpMemberDecorate %S 2 Offset 12
// CHECK-NEXT: OpMemberDecorate %S 3 Offset 24

// CHECK:      OpMemberDecorate %T 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %T 1 Offset 8
// CHECK-NEXT: OpMemberDecorate %T 2 Offset 32
// CHECK-NEXT: OpMemberDecorate %T 2 MatrixStride 12
// CHECK-NEXT: OpMemberDecorate %T 2 RowMajor
// CHECK-NEXT: OpMemberDecorate %T 3 Offset 56
// CHECK-NEXT: OpMemberDecorate %T 4 Offset 84
// CHECK-NEXT: OpMemberDecorate %T 5 Offset 88

// CHECK:      OpMemberDecorate %type_MyCBuffer 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 1 Offset 4
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 2 Offset 8
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 3 Offset 20
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 3 MatrixStride 12
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 3 ColMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 4 Offset 44
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 4 MatrixStride 8
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 4 RowMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 5 Offset 68
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 6 Offset 76
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 6 MatrixStride 12
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 6 ColMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 7 Offset 148
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 7 MatrixStride 8
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 7 RowMajor
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 8 Offset 216
// CHECK-NEXT: OpMemberDecorate %type_MyCBuffer 9 Offset 308
// CHECK-NEXT: OpDecorate %type_MyCBuffer Block

float main() : A {
    return 1.0;
}
