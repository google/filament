RWByteAddressBuffer prevent_dce : register(u0);

float4 dpdxCoarse_c28641() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddx_coarse(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdxCoarse_c28641()));
  return;
}
