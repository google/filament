// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external -Zpr %s | FileCheck %s

// CHECK: call void @"\01?GetMat
// CHECK-NOT: undef
// CHECK: load <16 x float>

void GetMat(out float4x4 mat);

[shader("pixel")]
float4 test(uint i:I) : SV_Target {
  float4x4 mat;
  GetMat(mat);
  return mat[i];
}
