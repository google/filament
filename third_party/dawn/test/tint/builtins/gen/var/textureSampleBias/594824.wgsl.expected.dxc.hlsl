RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float4 textureSampleBias_594824() {
  float3 arg_2 = (1.0f).xxx;
  float arg_3 = 1.0f;
  float4 res = arg_0.SampleBias(arg_1, arg_2, clamp(arg_3, -16.0f, 15.99f), int3((1).xxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBias_594824()));
  return;
}
