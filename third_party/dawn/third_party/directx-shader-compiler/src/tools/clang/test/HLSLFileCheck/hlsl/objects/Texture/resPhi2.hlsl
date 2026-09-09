// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

// CHECK: main

float4 Tex2D(Texture2D<float4> t,
  SamplerState s, float2 c) {
  return t.Sample(s, c);
}

SamplerState g_ss;
Texture2D<float4> g_texture[6];

float4 main(float2 c: T) : SV_Target {
  Texture2D<float4> texture;
  float4 r = 0;
  for (uint x=0;x<2;x++) {
    texture = g_texture[x];
    r += Tex2D(texture, g_ss, c);
  if (c.x>0)
    texture = g_texture[0];
  else
    texture = g_texture[2];

    r += Tex2D(texture, g_ss, c);
  }
  return r;
}
