// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK:attribute 'shader' must have one of these values: compute,vertex,pixel,hull,domain,geometry

[shader("lib")]
float4 ps_main(float4 a : A) : SV_TARGET
{
  return a;
}
