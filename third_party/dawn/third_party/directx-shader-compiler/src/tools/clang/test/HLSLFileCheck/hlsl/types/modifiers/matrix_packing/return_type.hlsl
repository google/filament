// RUN: %dxc /Tvs_6_0 /Evs_main  %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 2.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 3.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 4.000000e+00)

#pragma pack_matrix (row_major)

float2x2 GetMatrix()
{
 float2x2 mat = {1, 2, 3, 4};
 return mat;
}

float4 vs_main() : SV_POSITION
{
 float2x2 mat = GetMatrix();
 return float4(mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
}