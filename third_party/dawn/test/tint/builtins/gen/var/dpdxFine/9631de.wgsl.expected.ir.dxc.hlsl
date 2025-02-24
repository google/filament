
RWByteAddressBuffer prevent_dce : register(u0);
float2 dpdxFine_9631de() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdxFine_9631de()));
}

