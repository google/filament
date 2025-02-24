
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSample_6717ca() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  float2 v = arg_2;
  float4 res = arg_0.Sample(arg_1, float3(v, float(arg_3)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_6717ca()));
}

