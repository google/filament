RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float textureSample_c2f4e8() {
  float res = arg_0.Sample(arg_1, float4((1.0f).xxx, float(1))).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSample_c2f4e8()));
  return;
}
