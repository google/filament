// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// CHECK: expression is not assignable
// CHECK: expression is not assignable


float4 main(float2x4 mat: A, float4 b:B, float4 c:C) : SV_Target {
  (mat + mat)[1] = (mat * mat)[0];
  (b+c)[2] = c.y;
  return mat[0] + mat[1];
}