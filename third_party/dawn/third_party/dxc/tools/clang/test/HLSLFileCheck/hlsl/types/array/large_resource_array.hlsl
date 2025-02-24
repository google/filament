// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: ; tex                               texture     f32          2d      T0             t0 99999

Texture2D<float4> tex[99999];

float4 main(uint i : I) : SV_TARGET
{
  return tex[i][uint2(0,0)];
}
