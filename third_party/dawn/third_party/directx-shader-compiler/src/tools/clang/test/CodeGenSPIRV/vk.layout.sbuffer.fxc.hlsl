// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// fxc layout:
//
//   struct S
//   {
//
//       float scalar1;                 // Offset:    0
//       double scalar2;                // Offset:    8
//       float3 vector1;                // Offset:   16
//       float3 vector2;                // Offset:   28
//       double3 vector3;               // Offset:   40
//       double3 vector4;               // Offset:   64
//       float2x3 matrix1;              // Offset:   88
//       row_major float2x3 matrix2;    // Offset:  112
//       float3x2 matrix3;              // Offset:  136
//       row_major float3x2 matrix4;    // Offset:  160
//       float scalarArray1;            // Offset:  184
//       double scalarArray2[2];        // Offset:  192
//       float3 vectorArray1;           // Offset:  208
//       float3 vectorArray2[2];        // Offset:  220
//       double3 vectorArray3[3];       // Offset:  248
//       double3 vectorArray4[4];       // Offset:  320
//       float2x3 matrixArray1;         // Offset:  416
//       row_major float2x3 matrixArray2[2];// Offset:  440
//       float3x2 matrixArray3[3];      // Offset:  488
//       row_major float3x2 matrixArray4[4];// Offset:  560
//
//   }                                  // Size:   656

// CHECK: OpDecorate %_arr_float_uint_1 ArrayStride 4
// CHECK: OpDecorate %_arr_double_uint_2 ArrayStride 8
// CHECK: OpDecorate %_arr_v3float_uint_1 ArrayStride 12
// CHECK: OpDecorate %_arr_v3float_uint_2 ArrayStride 12
// CHECK: OpDecorate %_arr_v3double_uint_3 ArrayStride 24
// CHECK: OpDecorate %_arr_v3double_uint_4 ArrayStride 24
// CHECK: OpDecorate %_arr_mat2v3float_uint_1 ArrayStride 24
// CHECK: OpDecorate %_arr_mat2v3float_uint_2 ArrayStride 24
// CHECK: OpDecorate %_arr_mat3v2float_uint_3 ArrayStride 24
// CHECK: OpDecorate %_arr_mat3v2float_uint_4 ArrayStride 24

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 8
// CHECK: OpMemberDecorate %S 2 Offset 16
// CHECK: OpMemberDecorate %S 3 Offset 28
// CHECK: OpMemberDecorate %S 4 Offset 40
// CHECK: OpMemberDecorate %S 5 Offset 64
// CHECK: OpMemberDecorate %S 6 Offset 88
// CHECK: OpMemberDecorate %S 6 MatrixStride 8
// CHECK: OpMemberDecorate %S 6 RowMajor
// CHECK: OpMemberDecorate %S 7 Offset 112
// CHECK: OpMemberDecorate %S 7 MatrixStride 12
// CHECK: OpMemberDecorate %S 7 ColMajor
// CHECK: OpMemberDecorate %S 8 Offset 136
// CHECK: OpMemberDecorate %S 8 MatrixStride 12
// CHECK: OpMemberDecorate %S 8 RowMajor
// CHECK: OpMemberDecorate %S 9 Offset 160
// CHECK: OpMemberDecorate %S 9 MatrixStride 8
// CHECK: OpMemberDecorate %S 9 ColMajor
// CHECK: OpMemberDecorate %S 10 Offset 184
// CHECK: OpMemberDecorate %S 11 Offset 192
// CHECK: OpMemberDecorate %S 12 Offset 208
// CHECK: OpMemberDecorate %S 13 Offset 220
// CHECK: OpMemberDecorate %S 14 Offset 248
// CHECK: OpMemberDecorate %S 15 Offset 320
// CHECK: OpMemberDecorate %S 16 Offset 416
// CHECK: OpMemberDecorate %S 16 MatrixStride 8
// CHECK: OpMemberDecorate %S 16 RowMajor
// CHECK: OpMemberDecorate %S 17 Offset 440
// CHECK: OpMemberDecorate %S 17 MatrixStride 12
// CHECK: OpMemberDecorate %S 17 ColMajor
// CHECK: OpMemberDecorate %S 18 Offset 488
// CHECK: OpMemberDecorate %S 18 MatrixStride 12
// CHECK: OpMemberDecorate %S 18 RowMajor
// CHECK: OpMemberDecorate %S 19 Offset 560
// CHECK: OpMemberDecorate %S 19 MatrixStride 8
// CHECK: OpMemberDecorate %S 19 ColMajor
// CHECK: OpDecorate %_runtimearr_S ArrayStride 656

// fxc layout:
//
//   struct T
//   {
//
//       float scalar1;                 // Offset:    0
//
//       struct S
//       {
//
//           float scalar1;             // Offset:    8
//           double scalar2;            // Offset:   16
//           float3 vector1;            // Offset:   24
//           float3 vector2;            // Offset:   36
//           double3 vector3;           // Offset:   48
//           double3 vector4;           // Offset:   72
//           float2x3 matrix1;          // Offset:   96
//           row_major float2x3 matrix2;// Offset:  120
//           float3x2 matrix3;          // Offset:  144
//           row_major float3x2 matrix4;// Offset:  168
//           float scalarArray1;        // Offset:  192
//           double scalarArray2[2];    // Offset:  200
//           float3 vectorArray1;       // Offset:  216
//           float3 vectorArray2[2];    // Offset:  228
//           double3 vectorArray3[3];   // Offset:  256
//           double3 vectorArray4[4];   // Offset:  328
//           float2x3 matrixArray1;     // Offset:  424
//           row_major float2x3 matrixArray2[2];// Offset:  448
//           float3x2 matrixArray3[3];  // Offset:  496
//           row_major float3x2 matrixArray4[4];// Offset:  568
//
//       } sArray[2];                   // Offset:    8
//       float scalarArray1;            // Offset: 1320
//       float3 vectorArray1;           // Offset: 1324
//       float3 vector1;                // Offset: 1336
//       float2x3 matrixArray1;         // Offset: 1348
//       float3 vector2;                // Offset: 1372
//       double scalarArray2[2];        // Offset: 1384
//       float2x3 matrix1;              // Offset: 1400
//       double3 vector3;               // Offset: 1424
//       row_major float2x3 matrix2;    // Offset: 1448
//       row_major float2x3 matrixArray2[2];// Offset: 1472
//       float3x2 matrix3;              // Offset: 1520
//       row_major float3x2 matrix4;    // Offset: 1544
//       float3 vectorArray2[2];        // Offset: 1568
//       float3x2 matrixArray3[3];      // Offset: 1592
//       double3 vectorArray3[3];       // Offset: 1664
//       row_major float3x2 matrixArray4[4];// Offset: 1736
//       double3 vector4;               // Offset: 1832
//       double3 vectorArray4[4];       // Offset: 1856
//       double scalar2;                // Offset: 1952
//
//   }                                  // Size:  1960

// CHECK: OpDecorate %_arr_S_uint_2 ArrayStride 656

// CHECK: OpMemberDecorate %T 0 Offset 0
// CHECK: OpMemberDecorate %T 1 Offset 8
// CHECK: OpMemberDecorate %T 2 Offset 1320
// CHECK: OpMemberDecorate %T 3 Offset 1324
// CHECK: OpMemberDecorate %T 4 Offset 1336
// CHECK: OpMemberDecorate %T 5 Offset 1348
// CHECK: OpMemberDecorate %T 5 MatrixStride 8
// CHECK: OpMemberDecorate %T 5 RowMajor
// CHECK: OpMemberDecorate %T 6 Offset 1372
// CHECK: OpMemberDecorate %T 7 Offset 1384
// CHECK: OpMemberDecorate %T 8 Offset 1400
// CHECK: OpMemberDecorate %T 8 MatrixStride 8
// CHECK: OpMemberDecorate %T 8 RowMajor
// CHECK: OpMemberDecorate %T 9 Offset 1424
// CHECK: OpMemberDecorate %T 10 Offset 1448
// CHECK: OpMemberDecorate %T 10 MatrixStride 12
// CHECK: OpMemberDecorate %T 10 ColMajor
// CHECK: OpMemberDecorate %T 11 Offset 1472
// CHECK: OpMemberDecorate %T 11 MatrixStride 12
// CHECK: OpMemberDecorate %T 11 ColMajor
// CHECK: OpMemberDecorate %T 12 Offset 1520
// CHECK: OpMemberDecorate %T 12 MatrixStride 12
// CHECK: OpMemberDecorate %T 12 RowMajor
// CHECK: OpMemberDecorate %T 13 Offset 1544
// CHECK: OpMemberDecorate %T 13 MatrixStride 8
// CHECK: OpMemberDecorate %T 13 ColMajor
// CHECK: OpMemberDecorate %T 14 Offset 1568
// CHECK: OpMemberDecorate %T 15 Offset 1592
// CHECK: OpMemberDecorate %T 15 MatrixStride 12
// CHECK: OpMemberDecorate %T 15 RowMajor
// CHECK: OpMemberDecorate %T 16 Offset 1664
// CHECK: OpMemberDecorate %T 17 Offset 1736
// CHECK: OpMemberDecorate %T 17 MatrixStride 8
// CHECK: OpMemberDecorate %T 17 ColMajor
// CHECK: OpMemberDecorate %T 18 Offset 1832
// CHECK: OpMemberDecorate %T 19 Offset 1856
// CHECK: OpMemberDecorate %T 20 Offset 1952

// CHECK: OpDecorate %_runtimearr_T ArrayStride 1960

struct S {
    float              scalar1;
    double             scalar2;
    float3             vector1;
    float3             vector2;
    double3            vector3;
    double3            vector4;
    float2x3           matrix1;
    row_major float2x3 matrix2;
    float3x2           matrix3;
    row_major float3x2 matrix4;
    float              scalarArray1[1];
    double             scalarArray2[2];
    float3             vectorArray1[1];
    float3             vectorArray2[2];
    double3            vectorArray3[3];
    double3            vectorArray4[4];
    float2x3           matrixArray1[1];
    row_major float2x3 matrixArray2[2];
    float3x2           matrixArray3[3];
    row_major float3x2 matrixArray4[4];
};

struct T {
    float              scalar1;
    S                  sArray[2];
    float              scalarArray1[1];
    float3             vectorArray1[1];
    float3             vector1;
    float2x3           matrixArray1[1];
    float3             vector2;
    double             scalarArray2[2];
    float2x3           matrix1;
    double3            vector3;
    row_major float2x3 matrix2;
    row_major float2x3 matrixArray2[2];
    float3x2           matrix3;
    row_major float3x2 matrix4;
    float3             vectorArray2[2];
    float3x2           matrixArray3[3];
    double3            vectorArray3[3];
    row_major float3x2 matrixArray4[4];
    double3            vector4;
    double3            vectorArray4[4];
    double             scalar2;
};


StructuredBuffer<S> MySBuffer1;
StructuredBuffer<T> MySBuffer2;

float4 main() : SV_Target {
    return MySBuffer1[0].scalar1 + MySBuffer2[0].scalar1;
}
