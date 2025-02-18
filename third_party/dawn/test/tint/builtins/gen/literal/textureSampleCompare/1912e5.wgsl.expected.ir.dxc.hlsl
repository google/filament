
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float textureSampleCompare_1912e5() {
  float res = arg_0.SampleCmp(arg_1, float4((1.0f).xxx, float(1u)), 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_1912e5()));
}

