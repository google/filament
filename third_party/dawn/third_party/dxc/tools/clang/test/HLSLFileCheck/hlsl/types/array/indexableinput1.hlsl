// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float main(float2 a[4] : A, float2 b : B, float2 c[4] : C, int d : D/*, uint SI : SV_SampleIndex*/) : SV_TARGET
{
  return a[2].y + c[d].x + b.x;
}
