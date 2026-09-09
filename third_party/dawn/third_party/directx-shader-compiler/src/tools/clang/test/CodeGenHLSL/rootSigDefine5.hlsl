// RUN: %dxc -E main -T ps_6_0 -rootsig-define RS %s
// Test root signature define concatenated string

#define RS "DescriptorTable" \
           "(SRV(t3))"

Texture1D<float> tex : register(t3);
float main(float i : I) : SV_Target
{
  return tex[i];
}

