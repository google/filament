// RUN: %dxc -E main -T ps_6_0 -Gis %s | FileCheck %s

// Make sure mark precise on dxil operations when set Gis.
// CHECK:dx.precise

float main(float2 a:A) : SV_Target {
  return dot(a,a);
}