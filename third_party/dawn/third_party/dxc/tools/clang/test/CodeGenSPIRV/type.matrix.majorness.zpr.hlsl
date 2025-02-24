// RUN: %dxc -T vs_6_0 -E main /Zpr -fcgl  %s -spirv | FileCheck %s

struct S {
// CHECK: OpMemberDecorate %S 0 ColMajor
               float2x3 mat1[2];
// CHECK: OpMemberDecorate %S 1 ColMajor
  row_major    float2x3 mat2[2];
// CHECK: OpMemberDecorate %S 2 RowMajor
  column_major float2x3 mat3[2];
               float    f;
};

cbuffer MyCBuffer {
// CHECK: OpMemberDecorate %type_MyCBuffer 0 ColMajor
               float2x3 field1;
// CHECK: OpMemberDecorate %type_MyCBuffer 1 ColMajor
  row_major    float2x3 field2;
// CHECK: OpMemberDecorate %type_MyCBuffer 2 RowMajor
  column_major float2x3 field3;
               S        field4;
}

struct T {
               float    f[2]; // Make sure that arrays of non-matrices work
// CHECK: OpMemberDecorate %T 1 ColMajor
               float2x3 mat1;
// CHECK: OpMemberDecorate %T 2 ColMajor
  row_major    float2x3 mat2;
// CHECK: OpMemberDecorate %T 3 RowMajor
  column_major float2x3 mat3;
};

struct U {
               T        t;
// CHECK: OpMemberDecorate %U 1 ColMajor
               float2x3 mat1[2];
// CHECK: OpMemberDecorate %U 2 ColMajor
  row_major    float2x3 mat2[2];
// CHECK: OpMemberDecorate %U 3 RowMajor
  column_major float2x3 mat3[2];
               float    f;
};


RWStructuredBuffer<U> MySBuffer;

// CHECK: OpDecorate %_runtimearr_mat2v3float ArrayStride 32
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat2v3float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat2v3float 0 ColMajor
       StructuredBuffer<float2x3> ROSB1;
// CHECK: OpDecorate %_runtimearr_mat3v2float ArrayStride 24
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v2float 0 MatrixStride 8
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v2float 0 ColMajor
     RWStructuredBuffer<float3x2> RWSB1;
// CHECK: OpDecorate %_runtimearr_mat4v3float ArrayStride 64
// CHECK: OpMemberDecorate %type_AppendStructuredBuffer_mat4v3float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_AppendStructuredBuffer_mat4v3float 0 ColMajor
 AppendStructuredBuffer<float4x3> ASB1;
// CHECK: OpDecorate %_runtimearr_mat3v4float ArrayStride 48
// CHECK: OpMemberDecorate %type_ConsumeStructuredBuffer_mat3v4float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_ConsumeStructuredBuffer_mat3v4float 0 ColMajor
ConsumeStructuredBuffer<float3x4> CSB1;

// NOTE: -Zpr does not override explicit matrix orientation specified for the cases below.

// CHECK: OpDecorate %_runtimearr_mat3v4float_0 ArrayStride 64
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat3v4float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat3v4float 0 RowMajor
  StructuredBuffer<column_major float3x4> ROSB2;

// CHECK: OpDecorate %_runtimearr_mat3v3float ArrayStride 48
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v3float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v3float 0 ColMajor
RWStructuredBuffer<row_major float3x3> RWSB2;

float3 main() : A {
  return MySBuffer[0].mat1[1][1];
}
