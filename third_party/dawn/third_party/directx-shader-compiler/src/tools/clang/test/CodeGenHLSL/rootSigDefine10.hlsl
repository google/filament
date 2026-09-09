// RUN: %dxc -E main -T ps_6_0 -rootsig-define RS %s | FileCheck %s
// Test root signature define error: missing descriptor range.

// CHECK: error: validation errors
// CHECK: Root Signature in DXIL container is not compatible with shader
#define RS DescriptorTable(SRV(t0))

Texture1D<float> tex : register(t3);
float main(float i : I) : SV_Target
{
  return tex[i];
}

