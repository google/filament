// RUN: %dxc -Zi -E main -Od -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_DB
// RUN: %dxc -E main -Od -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_NODB

// CHK_DB: 9:10: error: local resource not guaranteed to map to unique global resource.
// CHK_NODB: 9:10: error: local resource not guaranteed to map to unique global resource.

float4 Tex2D(Texture2D<float4> t,
  SamplerState s, float2 c) {
  return t.Sample(s, c);
}

SamplerState g_ss;

float4 main(float2 c: T) : SV_Target {
Texture2D<float4> g_texture;
  return Tex2D(g_texture, g_ss, c);
}
