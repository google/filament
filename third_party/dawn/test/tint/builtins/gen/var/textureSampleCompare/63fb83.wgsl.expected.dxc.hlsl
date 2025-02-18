RWByteAddressBuffer prevent_dce : register(u0);
TextureCube arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);

float textureSampleCompare_63fb83() {
  float3 arg_2 = (1.0f).xxx;
  float arg_3 = 1.0f;
  float res = arg_0.SampleCmp(arg_1, arg_2, arg_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_63fb83()));
  return;
}
