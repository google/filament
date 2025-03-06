// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
Texture2D<float4> g_texture;
SamplerState g_ss;
float4 main() : SV_Target {
  return g_texture.Sample(g_ss, 1, 2);
}
