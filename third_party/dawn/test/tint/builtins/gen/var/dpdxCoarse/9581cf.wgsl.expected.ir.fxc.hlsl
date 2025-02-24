
RWByteAddressBuffer prevent_dce : register(u0);
float2 dpdxCoarse_9581cf() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx_coarse(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdxCoarse_9581cf()));
}

