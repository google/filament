RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float4 textureSample_2149ec() {
  float3 arg_2 = (1.0f).xxx;
  float4 res = arg_0.Sample(arg_1, arg_2, int3((1).xxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_2149ec()));
  return;
}
