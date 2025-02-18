[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

Texture2D<float4> ext_tex_plane_1 : register(t3);
cbuffer cbuffer_ext_tex_params : register(b4) {
  uint4 ext_tex_params[17];
};
SamplerState tint_symbol : register(s1);
Texture2D<float4> randomTexture : register(t1);
Texture2D<float4> depthTexture : register(t2);
