// RUN: %dxc -T ps_6_0 -E main -Zpc -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_runtimearr_mat2v3float ArrayStride 24
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat2v3float 0 MatrixStride 8
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat2v3float 0 RowMajor
       StructuredBuffer<float2x3> ROSB1;
// CHECK: OpDecorate %_runtimearr_mat3v2float ArrayStride 32
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v2float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v2float 0 RowMajor
     RWStructuredBuffer<float3x2> RWSB1;
// CHECK: OpDecorate %_runtimearr_mat4v3float ArrayStride 48
// CHECK: OpMemberDecorate %type_AppendStructuredBuffer_mat4v3float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_AppendStructuredBuffer_mat4v3float 0 RowMajor
 AppendStructuredBuffer<float4x3> ASB1;
// CHECK: OpDecorate %_runtimearr_mat3v4float ArrayStride 64
// CHECK: OpMemberDecorate %type_ConsumeStructuredBuffer_mat3v4float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_ConsumeStructuredBuffer_mat3v4float 0 RowMajor
ConsumeStructuredBuffer<float3x4> CSB1;

// NOTE: -Zpc does not override explicit matrix orientation specified for the cases below.

// CHECK: OpDecorate %_runtimearr_mat4v4float ArrayStride 64
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat4v4float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_StructuredBuffer_mat4v4float 0 ColMajor
     StructuredBuffer<row_major float4x4> ROSB2;

// CHECK: OpDecorate %_runtimearr_mat3v3float ArrayStride 48
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v3float 0 MatrixStride 16
// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v3float 0 RowMajor
RWStructuredBuffer<column_major float3x3> RWSB2;

float4 main() : SV_Target {
  return ROSB1[0][0][0] + ROSB2[0][0][0] + RWSB2[0][0][0];
}
