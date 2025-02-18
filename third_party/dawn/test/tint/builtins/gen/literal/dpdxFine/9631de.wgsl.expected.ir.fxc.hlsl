
RWByteAddressBuffer prevent_dce : register(u0);
float2 dpdxFine_9631de() {
  float2 res = ddx_fine((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdxFine_9631de()));
}

