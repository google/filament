
RWByteAddressBuffer prevent_dce : register(u0);
TextureCube<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBias_53b9f7() {
  float4 res = arg_0.SampleBias(arg_1, (1.0f).xxx, clamp(1.0f, -16.0f, 15.9899997711181640625f));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_53b9f7()));
}

