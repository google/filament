
RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSample_6e64fb() {
  float arg_2 = 1.0f;
  float4 res = arg_0.Sample(arg_1, arg_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_6e64fb()));
}

