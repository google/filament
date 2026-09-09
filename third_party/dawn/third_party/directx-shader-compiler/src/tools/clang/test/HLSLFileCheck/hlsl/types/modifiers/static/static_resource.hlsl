// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float4 Tex2D(Texture2D<float4> t,
  SamplerState s, float2 c) {
  return t.Sample(s, c);
}

SamplerState g_ss;
Texture2D<float4> g_texture;

static SamplerState s = g_ss;
static Texture2D<float4> t = g_texture;

float4 main(float2 c: T) : SV_Target {

  return Tex2D(t, s, c);
}
