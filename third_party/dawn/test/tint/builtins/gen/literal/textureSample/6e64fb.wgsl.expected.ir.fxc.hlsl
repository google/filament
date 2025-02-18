
RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSample_6e64fb() {
  float4 res = arg_0.Sample(arg_1, 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_6e64fb()));
}

