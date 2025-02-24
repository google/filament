// RUN: %dxc -T ps_6_1 -E main %s | FileCheck %s

// Make sure add is not sink into if.
// CHECK: fadd
// CHECK: fadd
// CHECK: fcmp
// CHECK-NEXT: br

Texture2D<float4> tex;
SamplerState s;
float4 main(float2 a:A, float b:B) : SV_Target {

  float2 coord = a + b;
  float4 c = b;
  if (b > 2) {
    c += tex.Sample(s, coord);
  }
  return c;

}