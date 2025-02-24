// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float main(float2 a : A, int b : B) : SV_Target
{
  float x;
  [branch]
  if (b == 1)
    x = a.x + 5.;
  else
    x = a.y - 77.;
  return x;
}
