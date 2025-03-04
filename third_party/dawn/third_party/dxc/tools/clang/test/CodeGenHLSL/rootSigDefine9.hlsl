// RUN: %dxc -E main -T ps_6_0 -rootsig-define RS %s | FileCheck %s
// Test root signature define error: syntax error in root signature.

// CHECK: root signature error - Unexpected token 'XescriptorTable' when parsing root signature
#define RS XescriptorTable(SRV(x))

Texture1D<float> tex : register(t3);
float main(float i : I) : SV_Target
{
  return tex[i];
}

