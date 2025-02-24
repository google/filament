// RUN: %dxc -E main -T ps_6_0 -rootsig-define RS %s
// Test root signature define overrides root signature attribute.
// Put an invalid root signature in the attribute and a valid one in the
// define. If compilation succeeds then the valid root signature was used.

Texture1D<float> tex : register(t3);

#define RS DescriptorTable(SRV(t3))
[RootSignature("SRV(t0)")]
float main(float i : I) : SV_Target
{
  return tex[i];
}

