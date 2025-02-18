
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float textureSampleCompare_af1051() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  float2 v = arg_2;
  float v_1 = arg_4;
  float res = arg_0.SampleCmp(arg_1, float3(v, float(arg_3)), v_1, (int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompare_af1051()));
}

