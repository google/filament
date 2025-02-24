
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSample_7fd8cb() {
  float res = arg_0.Sample(arg_1, float4((1.0f).xxx, float(1u))).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSample_7fd8cb()));
}

