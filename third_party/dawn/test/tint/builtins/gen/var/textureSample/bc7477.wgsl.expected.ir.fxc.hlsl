
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSample_bc7477() {
  float3 arg_2 = (1.0f).xxx;
  uint arg_3 = 1u;
  float3 v = arg_2;
  float4 res = arg_0.Sample(arg_1, float4(v, float(arg_3)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSample_bc7477()));
}

