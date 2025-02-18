
RWByteAddressBuffer prevent_dce : register(u0);
TextureCube<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSample_e53267() {
  float4 res = arg_0.Sample(arg_1, (1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_e53267()));
}

