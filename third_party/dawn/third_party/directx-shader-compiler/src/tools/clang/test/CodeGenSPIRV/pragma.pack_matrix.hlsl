// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s


// CHECK: OpMemberDecorate %type_StructuredBuffer_mat2v3float 0 ColMajor
#pragma pack_matrix(row_major)
StructuredBuffer<float2x3> ROSB1;

// CHECK: OpMemberDecorate %type_RWStructuredBuffer_mat3v2float 0 RowMajor
#pragma pack_matrix(column_major)
RWStructuredBuffer<float3x2> RWSB1;

// CHECK: OpMemberDecorate %type_AppendStructuredBuffer_mat4v3float 0 ColMajor
#pragma pack_matrix(row_major)
 AppendStructuredBuffer<float4x3> ASB1;

// CHECK: OpMemberDecorate %type_ConsumeStructuredBuffer_mat3v4float 0 RowMajor
#pragma pack_matrix(column_major)
ConsumeStructuredBuffer<float3x4> CSB1;

#pragma pack_matrix(row_major)
struct S {
// CHECK: OpMemberDecorate %S 0 ColMajor
               float2x3 sMat0;
// CHECK: OpMemberDecorate %S 1 ColMajor
     row_major float2x3 sMat1;
// CHECK: OpMemberDecorate %S 2 RowMajor
  column_major float2x3 sMat2;
};
RWStructuredBuffer<S> RWSB_S;

#pragma pack_matrix(column_major)
struct T {
// CHECK: OpMemberDecorate %T 0 RowMajor
               float2x3 tMat0;
// CHECK: OpMemberDecorate %T 1 ColMajor
  row_major    float2x3 tMat1;
// CHECK: OpMemberDecorate %T 2 RowMajor
  column_major float2x3 tMat2;
};
RWStructuredBuffer<T> RWSB_T;

// CHECK: OpMemberDecorate %type__Globals 0 ColMajor
#pragma pack_matrix(row_major)
float2x2 globalMat_1;

// CHECK: OpMemberDecorate %type__Globals 1 RowMajor
#pragma pack_matrix(column_major)
float2x2 globalMat_2;

// CHECK: OpMemberDecorate %type_StructuredBuffer_mat3v3float 0 ColMajor
StructuredBuffer<row_major float3x3> ExplicitOrientationRowMajor;

// CHECK: OpMemberDecorate %type_StructuredBuffer_mat4v4float 0 RowMajor
StructuredBuffer<column_major float4x4> ExplicitOrientationColMajor;

float4 main() : SV_Target {
  return 0.xxxx;
}

