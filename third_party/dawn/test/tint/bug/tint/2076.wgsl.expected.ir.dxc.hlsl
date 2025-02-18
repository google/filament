
SamplerState v : register(s1);
Texture2D<float4> randomTexture_plane0 : register(t1);
Texture2D<float4> randomTexture_plane1 : register(t3);
cbuffer cbuffer_randomTexture_params : register(b4) {
  uint4 randomTexture_params[17];
};
Texture2D<float4> depthTexture : register(t2);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

