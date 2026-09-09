// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// TODO: change float4 a[4] to float4x4 a
// CHECK: @main
float main(float4 a[4] : A, int4 b : B) : SV_Target
{
  return a[b.x][b.y];
}
