
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSample_6717ca() {
  float4 res = arg_0.Sample(arg_1, float3((1.0f).xx, float(int(1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_6717ca()));
}

