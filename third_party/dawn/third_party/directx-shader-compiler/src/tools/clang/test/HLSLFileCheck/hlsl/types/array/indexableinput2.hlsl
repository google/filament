// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float main(float2 a[3] : A, float2 b : B, float2 c[6] : C, int d : D, uint SI : SV_SampleIndex) : SV_TARGET
{
  return a[2].y + a[SI].x + c[d].x + b.x;
}
