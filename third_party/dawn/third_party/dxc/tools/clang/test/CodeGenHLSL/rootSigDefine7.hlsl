// RUN: %dxc -E main -T ps_6_0 -rootsig-define RS %s
// Test root signature define expanded macro concatenated string.

#define YYY "DescriptorTable" "(SRV(t3))"
#define RS YYY
           

Texture1D<float> tex : register(t3);
float main(float i : I) : SV_Target
{
  return tex[i];
}

