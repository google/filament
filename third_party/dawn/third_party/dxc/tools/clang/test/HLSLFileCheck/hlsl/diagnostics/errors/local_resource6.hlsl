// RUN: %dxc -Zi -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_DB
// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_NODB

// CHK_DB: 18:7: error: local resource not guaranteed to map to unique global resource.
// CHK_NODB: 18:7: error: local resource not guaranteed to map to unique global resource.

float4 Tex2D(Texture2D<float4> t,
  SamplerState s, float2 c) {
  return t.Sample(s, c);
}

SamplerState g_ss;
Texture2D<float4> g_texture;
Texture2D<float4> g_texture2;

float4 main(float2 c: T) : SV_Target {
Texture2D<float4> l_texture;
  if (c.x>0)
    l_texture = g_texture;
  else
    l_texture = g_texture2;

  return Tex2D(l_texture, g_ss, c);
}
