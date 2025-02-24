// RUN: %dxc -ast-dump /Tvs_6_0 /Evs_main  %s | FileCheck %s

// CHECK: GetMatrix 'row_major float2x2 (int)'

#pragma pack_matrix (row_major)

float2x2 GetMatrix(int i)
{
 if(i > 0)
 {
   #pragma pack_matrix (column_major)
   float2x2 mat = {1, 2, 3, 4};
   return mat;
 }
 else
 {
   #pragma pack_matrix (row_major)
   float2x2 mat = {2, 3, 3, 4};
   return mat;
 }
}

float4 vs_main() : SV_POSITION
{
 float2x2 mat = GetMatrix(1);
 return float4(mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
}