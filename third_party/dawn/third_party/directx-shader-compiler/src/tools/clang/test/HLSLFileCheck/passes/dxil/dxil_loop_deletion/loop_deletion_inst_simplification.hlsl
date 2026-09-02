// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Test complicated loop elimination that requires intruction simplification
// to remove all the useless loops

// We are literally returning zero. There is no need of branches
// CHECK: @main()
// CHECK-NOT:br

Texture2D<float> g_tex : register(t0);
SamplerState g_samp : register(s0);

float4 main(float2 tangent : TANGENT0) : SV_Target
{
  float2 uv = 0;
  float cond = 0;
  for (uint i = 0; i < 16; i++) {
    cond = g_tex.SampleGrad(g_samp, uv, 0, 0);
    uv += tangent * cond;
  }
  if (cond < 0) {
    uv = 0.5f * (uv);
    for (uint i = 0; i < 6; i++) {
      cond = g_tex.SampleGrad(g_samp, uv, 0, 0);
      if (cond)
        break;
      if (cond > 0)
        uv += 1;
    }
  }

  float cond2 = 0;
  for (uint i = 0; i < 16; i++) {
    if (cond2 >= 3)
      break;
    cond2 += g_tex.SampleGrad(g_samp, 0, uv, uv);
  }

  return 0;
}
