// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure uav array can has globallycoherent.
// CHECK:i32 12, i32 2, i1 true

globallycoherent RWTexture2D<float> tex[12] : register(u1);

float main(float2 c:C) : SV_Target {
  return tex[0][c];
}