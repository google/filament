
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBias_eed7c4() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  float3 v = arg_2;
  float v_1 = clamp(arg_4, -16.0f, 15.9899997711181640625f);
  float4 res = arg_0.SampleBias(arg_1, float4(v, float(arg_3)), v_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_eed7c4()));
}

