// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: cannot Sample from resource containing uint

Texture2D<uint> tid;
SamplerState             g_s:     register(s0);
float4 main(float2 p: A) : SV_Target {
  return tid.Sample(g_s, p);
}
