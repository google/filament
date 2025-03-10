// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: !dx.precise

float4 main(float a : A) : SV_Target
{
  float psin = sin(a);

  return psin;
}
