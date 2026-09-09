// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: !dx.precise

float4 main(float a : A) : SV_Target
{
  precise float psin = sin(a);

  return psin;
}
