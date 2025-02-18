RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float4 textureSampleBias_c6953d() {
  float3 arg_2 = (1.0f).xxx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float4 res = arg_0.SampleBias(arg_1, float4(arg_2, float(arg_3)), clamp(arg_4, -16.0f, 15.99f));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_c6953d()));
  return;
}
