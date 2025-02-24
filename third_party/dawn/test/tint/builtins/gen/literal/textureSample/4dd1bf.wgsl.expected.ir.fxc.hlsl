
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSample_4dd1bf() {
  float4 res = arg_0.Sample(arg_1, float4((1.0f).xxx, float(int(1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_4dd1bf()));
}

