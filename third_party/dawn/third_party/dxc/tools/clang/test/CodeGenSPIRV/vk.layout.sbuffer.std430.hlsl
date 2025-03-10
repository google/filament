// RUN: %dxc -T ps_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

struct R {     // Alignment       Offset     Size       Next
    float2 rf; // 8(vec2)      -> 0        + 8(vec2)  = 8
};             // 8               8          8

struct S {      // Alignment    Offset                                Size        Next
    R      sf1; // 8         -> 0                                   + 8         = 8
    float  sf2; // 4         -> 8                                   + 4         = 12
    float3 sf3; // 16(vec4)  -> 16 (12 round up to vec4 alignment)  + 12(vec3)  = 28
    float  sf4; // 4         -> 28                                  + 4         = 32
};              // 16(max)                                                        32

struct T {                      // Alignment     Offset                               Size              = Next
               int      tf1;    // 4          -> 0                                  + 4                 = 4
               R        tf2[3]; // 8          -> 8                                  + 3 * stride(8)     = 32
               float3x2 tf3;    // 16(vec4)   -> 32 (32 round up to vec4 alignment) + 2 * stride(vec4)  = 64
  row_major    int3x2   tf4;    // 16(vec4)   -> 64 (64 round up to vec4 alignment) + 3 * stride(vec2)  = 88
               S        tf5;    // 16         -> 96 (88 round up to S alignment)    + 32                = 128
               float    tf6;    // 4          -> 128                                + 4                 = 132
};                              // 16(max)                                                                144(132 round up to T max alignment)

struct SBuffer {              // Alignment   Offset                                 Size                     Next
                 bool     a;     // 4        -> 0                                    +     4                  = 4
                 uint1    b;     // 4        -> 4                                    +     4                  = 8
                 float3   c;     // 16(vec4) -> 16 (8 round up to vec4 alignment)    + 3 * 4                  = 28
    row_major    float2x3 d;     // 16(vec4) -> 32 (28 round up to vec4 alignment)   + 2 * stride(vec4)       = 64
    column_major float2x3 e;     // 16(vec4) -> 64 (64 round up to vec2 alignment)   + 3 * stride(vec2)       = 88
                 float2x1 f;     // 8(vec2)  -> 88 (88 round up to vec2 aligment)    + 2 * 4                  = 96
    row_major    float2x3 g[3];  // 16(vec4) -> 96 (96 round up to vec4 alignment)   + 3 * 2 * stride(vec4)   = 192
    column_major float2x2 h[4];  // 16(vec4) -> 192 (192 round up to vec2 alignment) + 4 * 2 * stride(vec2)   = 256
    row_major    int2x3   i[5];  // 16(vec4) -> 256 (256 round up to vec4 alignment) + 5 * 2 * stride(vec4)   = 416
                 T        t;     // 16       -> 416 (416 round up to T alignment)    + 144                    = 560
                 float    z;     // 4        -> 560

};

StructuredBuffer<SBuffer> MySBuffer;

// CHECK:      OpDecorate %_arr_mat2v3float_uint_3 ArrayStride 32
// CHECK:      OpDecorate %_arr_mat2v2float_uint_4 ArrayStride 16
// CHECK:      OpDecorate %_arr_v3int_uint_2 ArrayStride 16
// CHECK:      OpDecorate %_arr__arr_v3int_uint_2_uint_5 ArrayStride 32

// CHECK:      OpMemberDecorate %R 0 Offset 0

// CHECK:      OpDecorate %_arr_R_uint_3 ArrayStride 8
// CEHCK:      OpDecorate %_arr_v2int_uint_3 ArrayStride 8

// CHECK:      OpMemberDecorate %S 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %S 1 Offset 8
// CHECK-NEXT: OpMemberDecorate %S 2 Offset 16
// CHECK-NEXT: OpMemberDecorate %S 3 Offset 28

// CHECK:      OpMemberDecorate %T 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %T 1 Offset 8
// CHECK-NEXT: OpMemberDecorate %T 2 Offset 32
// CHECK-NEXT: OpMemberDecorate %T 2 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %T 2 RowMajor
// CHECK-NEXT: OpMemberDecorate %T 3 Offset 64
// CHECK-NEXT: OpMemberDecorate %T 4 Offset 96
// CHECK-NEXT: OpMemberDecorate %T 5 Offset 128

// CHECK:      OpMemberDecorate %SBuffer 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %SBuffer 1 Offset 4
// CHECK-NEXT: OpMemberDecorate %SBuffer 2 Offset 16
// CHECK-NEXT: OpMemberDecorate %SBuffer 3 Offset 32
// CHECK-NEXT: OpMemberDecorate %SBuffer 3 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %SBuffer 3 ColMajor
// CHECK-NEXT: OpMemberDecorate %SBuffer 4 Offset 64
// CHECK-NEXT: OpMemberDecorate %SBuffer 4 MatrixStride 8
// CHECK-NEXT: OpMemberDecorate %SBuffer 4 RowMajor
// CHECK-NEXT: OpMemberDecorate %SBuffer 5 Offset 88
// CHECK-NEXT: OpMemberDecorate %SBuffer 6 Offset 96
// CHECK-NEXT: OpMemberDecorate %SBuffer 6 MatrixStride 16
// CHECK-NEXT: OpMemberDecorate %SBuffer 6 ColMajor
// CHECK-NEXT: OpMemberDecorate %SBuffer 7 Offset 192
// CHECK-NEXT: OpMemberDecorate %SBuffer 7 MatrixStride 8
// CHECK-NEXT: OpMemberDecorate %SBuffer 7 RowMajor
// CHECK-NEXT: OpMemberDecorate %SBuffer 8 Offset 256
// CHECK-NEXT: OpMemberDecorate %SBuffer 9 Offset 416
// CHECK-NEXT: OpMemberDecorate %SBuffer 10 Offset 560

// CHECK:      OpDecorate %_runtimearr_SBuffer ArrayStride 576

// CHECK:      OpMemberDecorate %type_StructuredBuffer_SBuffer 0 Offset 0
// CHECK-NEXT: OpMemberDecorate %type_StructuredBuffer_SBuffer 0 NonWritable
// CHECK-NEXT: OpDecorate %type_StructuredBuffer_SBuffer BufferBlock

float main() : SV_Target {
    return 1.0;
}
