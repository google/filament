// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s

// Test that one of the textures have been removed completely since they have
// the same register.

// CHECK: @main

SamplerState samp : register(s0);
Texture2D tex0 : register(t0);
Texture2D tex1 : register(t0); // Same register as tex1

cbuffer cb : register ( b0 ) {
  int2 g_count;
  int g_delta;
}

float foo(bool flag) {
  float result = 0;
  [loop]
  for (int y = 0; y < g_count.x; y += g_delta) {
    [loop]
    for (int x = 0; x < g_count.x; x += g_delta) {
      float2 uv = float2(int2 (x,y));
      float4 g;
      if (flag) {
        g = tex1.Gather(samp, uv, 1);
      }
      else {
        g = tex0.Gather(samp, uv, 1);
      }
      result = g.x;
    }
  }
  return result;
}

[RootSignature("CBV(b0),DescriptorTable(SRV(t0),SRV(t1)),DescriptorTable(Sampler(s0))")]
float main() : SV_Target {
  return foo(false);
}

