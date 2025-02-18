RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float4 textureSample_85c4ba() {
  float2 arg_2 = (1.0f).xx;
  float4 res = arg_0.Sample(arg_1, arg_2, int2((1).xx));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_85c4ba()));
  return;
}
