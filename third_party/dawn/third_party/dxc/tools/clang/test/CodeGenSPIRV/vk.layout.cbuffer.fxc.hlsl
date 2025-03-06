// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// fxc layout:
//
//   struct S
//   {
//
//       float scalar1;                 // Offset:    0
//       double scalar2;                // Offset:    8
//       float3 vector1;                // Offset:   16
//       float3 vector2;                // Offset:   32
//       double3 vector3;               // Offset:   48
//       double3 vector4;               // Offset:   80
//       float2x3 matrix1;              // Offset:  112
//       row_major float2x3 matrix2;    // Offset:  160
//       float3x2 matrix3;              // Offset:  192
//       row_major float3x2 matrix4;    // Offset:  224
//       float scalarArray1;            // Offset:  272
//       double scalarArray2[2];        // Offset:  288
//       float3 vectorArray1;           // Offset:  320
//       float3 vectorArray2[2];        // Offset:  336
//       double3 vectorArray3[3];       // Offset:  368
//       double3 vectorArray4[4];       // Offset:  464
//       float2x3 matrixArray1;         // Offset:  592
//       row_major float2x3 matrixArray2[2];// Offset:  640
//       float3x2 matrixArray3[3];      // Offset:  704
//       row_major float3x2 matrixArray4[4];// Offset:  800
//
//   }                                  // Size:   984

// CHECK: OpDecorate %_arr_float_uint_1 ArrayStride 16
// CHECK: OpDecorate %_arr_double_uint_2 ArrayStride 16
// CHECK: OpDecorate %_arr_v3float_uint_1 ArrayStride 16
// CHECK: OpDecorate %_arr_v3float_uint_2 ArrayStride 16
// CHECK: OpDecorate %_arr_v3double_uint_3 ArrayStride 32
// CHECK: OpDecorate %_arr_v3double_uint_4 ArrayStride 32
// CHECK: OpDecorate %_arr_mat2v3float_uint_1 ArrayStride 48
// CHECK: OpDecorate %_arr_mat2v3float_uint_2 ArrayStride 32
// CHECK: OpDecorate %_arr_mat3v2float_uint_3 ArrayStride 32
// CHECK: OpDecorate %_arr_mat3v2float_uint_4 ArrayStride 48

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 8
// CHECK: OpMemberDecorate %S 2 Offset 16
// CHECK: OpMemberDecorate %S 3 Offset 32
// CHECK: OpMemberDecorate %S 4 Offset 48
// CHECK: OpMemberDecorate %S 5 Offset 80
// CHECK: OpMemberDecorate %S 6 Offset 112
// CHECK: OpMemberDecorate %S 6 MatrixStride 16
// CHECK: OpMemberDecorate %S 6 RowMajor
// CHECK: OpMemberDecorate %S 7 Offset 160
// CHECK: OpMemberDecorate %S 7 MatrixStride 16
// CHECK: OpMemberDecorate %S 7 ColMajor
// CHECK: OpMemberDecorate %S 8 Offset 192
// CHECK: OpMemberDecorate %S 8 MatrixStride 16
// CHECK: OpMemberDecorate %S 8 RowMajor
// CHECK: OpMemberDecorate %S 9 Offset 224
// CHECK: OpMemberDecorate %S 9 MatrixStride 16
// CHECK: OpMemberDecorate %S 9 ColMajor
// CHECK: OpMemberDecorate %S 10 Offset 272
// CHECK: OpMemberDecorate %S 11 Offset 288
// CHECK: OpMemberDecorate %S 12 Offset 320
// CHECK: OpMemberDecorate %S 13 Offset 336
// CHECK: OpMemberDecorate %S 14 Offset 368
// CHECK: OpMemberDecorate %S 15 Offset 464
// CHECK: OpMemberDecorate %S 16 Offset 592
// CHECK: OpMemberDecorate %S 16 MatrixStride 16
// CHECK: OpMemberDecorate %S 16 RowMajor
// CHECK: OpMemberDecorate %S 17 Offset 640
// CHECK: OpMemberDecorate %S 17 MatrixStride 16
// CHECK: OpMemberDecorate %S 17 ColMajor
// CHECK: OpMemberDecorate %S 18 Offset 704
// CHECK: OpMemberDecorate %S 18 MatrixStride 16
// CHECK: OpMemberDecorate %S 18 RowMajor
// CHECK: OpMemberDecorate %S 19 Offset 800
// CHECK: OpMemberDecorate %S 19 MatrixStride 16
// CHECK: OpMemberDecorate %S 19 ColMajor
// CHECK: OpDecorate %_arr_S_uint_2 ArrayStride 992

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

// fxc layout:
//
// cbuffer MyCBuffer
// {
//
//   float CB_scalar1;                  // Offset:    0 Size:     4
//
//   struct S
//   {
//
//       float scalar1;                 // Offset:   16
//       double scalar2;                // Offset:   24
//       float3 vector1;                // Offset:   32
//       float3 vector2;                // Offset:   48
//       double3 vector3;               // Offset:   64
//       double3 vector4;               // Offset:   96
//       float2x3 matrix1;              // Offset:  128
//       row_major float2x3 matrix2;    // Offset:  176
//       float3x2 matrix3;              // Offset:  208
//       row_major float3x2 matrix4;    // Offset:  240
//       float scalarArray1;            // Offset:  288
//       double scalarArray2[2];        // Offset:  304
//       float3 vectorArray1;           // Offset:  336
//       float3 vectorArray2[2];        // Offset:  352
//       double3 vectorArray3[3];       // Offset:  384
//       double3 vectorArray4[4];       // Offset:  480
//       float2x3 matrixArray1;         // Offset:  608
//       row_major float2x3 matrixArray2[2];// Offset:  656
//       float3x2 matrixArray3[3];      // Offset:  720
//       row_major float3x2 matrixArray4[4];// Offset:  816
//
//   } CB_sArray[2];                    // Offset:   16 Size:  1976 [unused]
//   float CB_scalarArray1;             // Offset: 2000 Size:     4 [unused]
//   float3 CB_vectorArray1;            // Offset: 2016 Size:    12 [unused]
//   float3 CB_vector1;                 // Offset: 2032 Size:    12 [unused]
//   float2x3 CB_matrixArray1;          // Offset: 2048 Size:    40 [unused]
//   float3 CB_vector2;                 // Offset: 2096 Size:    12 [unused]
//   double CB_scalarArray2[2];         // Offset: 2112 Size:    24 [unused]
//   float2x3 CB_matrix1;               // Offset: 2144 Size:    40 [unused]
//   double3 CB_vector3;                // Offset: 2192 Size:    24 [unused]
//   row_major float2x3 CB_matrix2;     // Offset: 2224 Size:    28 [unused]
//   row_major float2x3 CB_matrixArray2[2];// Offset: 2256 Size:    60 [unused]
//   float3x2 CB_matrix3;               // Offset: 2320 Size:    28 [unused]
//   row_major float3x2 CB_matrix4;     // Offset: 2352 Size:    40 [unused]
//   float3 CB_vectorArray2[2];         // Offset: 2400 Size:    28 [unused]
//   float3x2 CB_matrixArray3[3];       // Offset: 2432 Size:    92 [unused]
//   double3 CB_vectorArray3[3];        // Offset: 2528 Size:    88 [unused]
//   row_major float3x2 CB_matrixArray4[4];// Offset: 2624 Size:   184 [unused]
//   double3 CB_vector4;                // Offset: 2816 Size:    24 [unused]
//   double3 CB_vectorArray4[4];        // Offset: 2848 Size:   120 [unused]
//   double CB_scalar2;                 // Offset: 2968 Size:     8 [unused]
//   float3 CB_vector5;                 // Offset: 2976 Size:    12 [unused]
//   int CB_scalarArray3[3];            // Offset: 2992 Size:    36 [unused]
//   float3 CB_vector6;                 // Offset: 3028 Size:    12 [unused]
//
// }

// CHECK: OpMemberDecorate %type_MyCBuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyCBuffer 1 Offset 16
// CHECK: OpMemberDecorate %type_MyCBuffer 2 Offset 2000
// CHECK: OpMemberDecorate %type_MyCBuffer 3 Offset 2016
// CHECK: OpMemberDecorate %type_MyCBuffer 4 Offset 2032
// CHECK: OpMemberDecorate %type_MyCBuffer 5 Offset 2048
// CHECK: OpMemberDecorate %type_MyCBuffer 5 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 5 RowMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 6 Offset 2096
// CHECK: OpMemberDecorate %type_MyCBuffer 7 Offset 2112
// CHECK: OpMemberDecorate %type_MyCBuffer 8 Offset 2144
// CHECK: OpMemberDecorate %type_MyCBuffer 8 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 8 RowMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 9 Offset 2192
// CHECK: OpMemberDecorate %type_MyCBuffer 10 Offset 2224
// CHECK: OpMemberDecorate %type_MyCBuffer 10 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 10 ColMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 11 Offset 2256
// CHECK: OpMemberDecorate %type_MyCBuffer 11 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 11 ColMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 12 Offset 2320
// CHECK: OpMemberDecorate %type_MyCBuffer 12 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 12 RowMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 13 Offset 2352
// CHECK: OpMemberDecorate %type_MyCBuffer 13 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 13 ColMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 14 Offset 2400
// CHECK: OpMemberDecorate %type_MyCBuffer 15 Offset 2432
// CHECK: OpMemberDecorate %type_MyCBuffer 15 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 15 RowMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 16 Offset 2528
// CHECK: OpMemberDecorate %type_MyCBuffer 17 Offset 2624
// CHECK: OpMemberDecorate %type_MyCBuffer 17 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 17 ColMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 18 Offset 2816
// CHECK: OpMemberDecorate %type_MyCBuffer 19 Offset 2848
// CHECK: OpMemberDecorate %type_MyCBuffer 20 Offset 2968
// CHECK: OpMemberDecorate %type_MyCBuffer 21 Offset 2976
// CHECK: OpMemberDecorate %type_MyCBuffer 22 Offset 2992
// CHECK: OpMemberDecorate %type_MyCBuffer 23 Offset 3028

cbuffer MyCBuffer
{
    float              CB_scalar1;
    S                  CB_sArray[2];
    float              CB_scalarArray1[1];
    float3             CB_vectorArray1[1];
    float3             CB_vector1;
    float2x3           CB_matrixArray1[1];
    float3             CB_vector2;
    double             CB_scalarArray2[2];
    float2x3           CB_matrix1;
    double3            CB_vector3;
    row_major float2x3 CB_matrix2;
    row_major float2x3 CB_matrixArray2[2];
    float3x2           CB_matrix3;
    row_major float3x2 CB_matrix4;
    float3             CB_vectorArray2[2];
    float3x2           CB_matrixArray3[3];
    double3            CB_vectorArray3[3];
    row_major float3x2 CB_matrixArray4[4];
    double3            CB_vector4;
    double3            CB_vectorArray4[4];
    double             CB_scalar2;
    float3             CB_vector5;
    int                CB_scalarArray3[3];
    float3             CB_vector6;
};

// fxc layout:
//
// tbuffer MyTBuffer
// {
//
//   float TB_scalar1;                  // Offset:    0 Size:     4
//
//   struct S
//   {
//
//       float scalar1;                 // Offset:   16
//       double scalar2;                // Offset:   24
//       float3 vector1;                // Offset:   32
//       float3 vector2;                // Offset:   48
//       double3 vector3;               // Offset:   64
//       double3 vector4;               // Offset:   96
//       float2x3 matrix1;              // Offset:  128
//       row_major float2x3 matrix2;    // Offset:  176
//       float3x2 matrix3;              // Offset:  208
//       row_major float3x2 matrix4;    // Offset:  240
//       float scalarArray1;            // Offset:  288
//       double scalarArray2[2];        // Offset:  304
//       float3 vectorArray1;           // Offset:  336
//       float3 vectorArray2[2];        // Offset:  352
//       double3 vectorArray3[3];       // Offset:  384
//       double3 vectorArray4[4];       // Offset:  480
//       float2x3 matrixArray1;         // Offset:  608
//       row_major float2x3 matrixArray2[2];// Offset:  656
//       float3x2 matrixArray3[3];      // Offset:  720
//       row_major float3x2 matrixArray4[4];// Offset:  816
//
//   } TB_sArray[2];                    // Offset:   16 Size:  1976 [unused]
//   float TB_scalarArray1;             // Offset: 2000 Size:     4 [unused]
//   float3 TB_vectorArray1;            // Offset: 2016 Size:    12 [unused]
//   float3 TB_vector1;                 // Offset: 2032 Size:    12 [unused]
//   float2x3 TB_matrixArray1;          // Offset: 2048 Size:    40 [unused]
//   float3 TB_vector2;                 // Offset: 2096 Size:    12 [unused]
//   double TB_scalarArray2[2];         // Offset: 2112 Size:    24 [unused]
//   float2x3 TB_matrix1;               // Offset: 2144 Size:    40 [unused]
//   double3 TB_vector3;                // Offset: 2192 Size:    24 [unused]
//   row_major float2x3 TB_matrix2;     // Offset: 2224 Size:    28 [unused]
//   row_major float2x3 TB_matrixArray2[2];// Offset: 2256 Size:    60 [unused]
//   float3x2 TB_matrix3;               // Offset: 2320 Size:    28 [unused]
//   row_major float3x2 TB_matrix4;     // Offset: 2352 Size:    40 [unused]
//   float3 TB_vectorArray2[2];         // Offset: 2400 Size:    28 [unused]
//   float3x2 TB_matrixArray3[3];       // Offset: 2432 Size:    92 [unused]
//   double3 TB_vectorArray3[3];        // Offset: 2528 Size:    88 [unused]
//   row_major float3x2 TB_matrixArray4[4];// Offset: 2624 Size:   184 [unused]
//   double3 TB_vector4;                // Offset: 2816 Size:    24 [unused]
//   double3 TB_vectorArray4[4];        // Offset: 2848 Size:   120 [unused]
//   double TB_scalar2;                 // Offset: 2968 Size:     8 [unused]
//
// }

// CHECK: OpMemberDecorate %type_MyTBuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyTBuffer 1 Offset 16
// CHECK: OpMemberDecorate %type_MyTBuffer 2 Offset 2000
// CHECK: OpMemberDecorate %type_MyTBuffer 3 Offset 2016
// CHECK: OpMemberDecorate %type_MyTBuffer 4 Offset 2032
// CHECK: OpMemberDecorate %type_MyTBuffer 5 Offset 2048
// CHECK: OpMemberDecorate %type_MyTBuffer 5 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 5 RowMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 6 Offset 2096
// CHECK: OpMemberDecorate %type_MyTBuffer 7 Offset 2112
// CHECK: OpMemberDecorate %type_MyTBuffer 8 Offset 2144
// CHECK: OpMemberDecorate %type_MyTBuffer 8 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 8 RowMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 9 Offset 2192
// CHECK: OpMemberDecorate %type_MyTBuffer 10 Offset 2224
// CHECK: OpMemberDecorate %type_MyTBuffer 10 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 10 ColMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 11 Offset 2256
// CHECK: OpMemberDecorate %type_MyTBuffer 11 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 11 ColMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 12 Offset 2320
// CHECK: OpMemberDecorate %type_MyTBuffer 12 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 12 RowMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 13 Offset 2352
// CHECK: OpMemberDecorate %type_MyTBuffer 13 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 13 ColMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 14 Offset 2400
// CHECK: OpMemberDecorate %type_MyTBuffer 15 Offset 2432
// CHECK: OpMemberDecorate %type_MyTBuffer 15 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 15 RowMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 16 Offset 2528
// CHECK: OpMemberDecorate %type_MyTBuffer 17 Offset 2624
// CHECK: OpMemberDecorate %type_MyTBuffer 17 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyTBuffer 17 ColMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 18 Offset 2816
// CHECK: OpMemberDecorate %type_MyTBuffer 19 Offset 2848
// CHECK: OpMemberDecorate %type_MyTBuffer 20 Offset 2968

tbuffer MyTBuffer
{
    float              TB_scalar1;
    S                  TB_sArray[2];
    float              TB_scalarArray1[1];
    float3             TB_vectorArray1[1];
    float3             TB_vector1;
    float2x3           TB_matrixArray1[1];
    float3             TB_vector2;
    double             TB_scalarArray2[2];
    float2x3           TB_matrix1;
    double3            TB_vector3;
    row_major float2x3 TB_matrix2;
    row_major float2x3 TB_matrixArray2[2];
    float3x2           TB_matrix3;
    row_major float3x2 TB_matrix4;
    float3             TB_vectorArray2[2];
    float3x2           TB_matrixArray3[3];
    double3            TB_vectorArray3[3];
    row_major float3x2 TB_matrixArray4[4];
    double3            TB_vector4;
    double3            TB_vectorArray4[4];
    double             TB_scalar2;
};

ConstantBuffer<S> MyConstantBuffer;
TextureBuffer<S> MyTextureBuffer;

float4 main() : SV_Target {
    return CB_scalar1 + TB_scalar1 + MyConstantBuffer.scalar1 + MyTextureBuffer.scalar1;
}
