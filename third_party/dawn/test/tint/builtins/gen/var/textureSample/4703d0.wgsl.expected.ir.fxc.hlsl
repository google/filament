
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSample_4703d0() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float2 v = arg_2;
  float res = arg_0.Sample(arg_1, float3(v, float(arg_3)), (int(1)).xx).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSample_4703d0()));
}

