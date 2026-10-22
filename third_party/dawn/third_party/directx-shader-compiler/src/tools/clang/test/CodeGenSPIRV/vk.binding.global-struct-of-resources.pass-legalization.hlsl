// RUN: %dxc -T ps_6_0 -E main -O0  %s -spirv | FileCheck %s

// CHECK: OpDecorate %g_CombinedTextureSampler_t DescriptorSet 0
// CHECK: OpDecorate %g_CombinedTextureSampler_t Binding 0
// CHECK: OpDecorate %g_CombinedTextureSampler_s DescriptorSet 0
// CHECK: OpDecorate %g_CombinedTextureSampler_s Binding 1

struct sampler2D {
  Texture2D t;
  SamplerState s;
};
float4 tex2D(sampler2D x, float2 v) { return x.t.Sample(x.s, v); }

struct v2f {
  float4 vertex : SV_POSITION;
  float2 uv : TEXCOORD0;
};

sampler2D g_CombinedTextureSampler;

float4 main(v2f i) : SV_Target {
  return tex2D(g_CombinedTextureSampler, i.uv);
}
