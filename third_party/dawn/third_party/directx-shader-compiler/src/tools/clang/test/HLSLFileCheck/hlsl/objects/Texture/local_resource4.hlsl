// RUN: %dxc -E main -Od -T ps_6_0 %s | FileCheck %s

// CHECK: main

float4 Tex2D(Texture2D<float4> t,
  SamplerState s, float2 c) {
  return t.Sample(s, c);
}


Texture2D<float4> g_texture;
SamplerState g_ss;


static Texture2D<float4> g_texture2;

float4 main(float2 c: T) : SV_Target {
  g_texture2 = g_texture;
  return Tex2D(g_texture2, g_ss, c);
}
