RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float textureSampleCompare_a3ca7e() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = 1;
  float arg_4 = 1.0f;
  float res = arg_0.SampleCmp(arg_1, float4(arg_2, float(arg_3)), arg_4);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_a3ca7e()));
  return;
}
