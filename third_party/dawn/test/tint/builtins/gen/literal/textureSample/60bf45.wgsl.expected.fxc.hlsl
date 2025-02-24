RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float textureSample_60bf45() {
  float res = arg_0.Sample(arg_1, float3((1.0f).xx, float(1)), int2((1).xx)).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSample_60bf45()));
  return;
}
