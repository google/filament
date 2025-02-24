// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Shader extensions for 11.1


// CHECK: Asin
// CHECK: Acos

float v;
uint2 s;
int2  i;

float4 main(float4 arg : A) : SV_TARGET {
  float t = asin(v) + acos(v) + s.x/s.y + i.x/i.y + log(v);
  t +=  ddx(v) + ddy(s.x);
  t +=  ddx_fine(v) + ddy_fine(s.x);
  return t;
}
