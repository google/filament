
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float textureSampleCompare_a3ca7e() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  float3 v = arg_2;
  float v_1 = arg_4;
  float res = arg_0.SampleCmp(arg_1, float4(v, float(arg_3)), v_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_a3ca7e()));
}

