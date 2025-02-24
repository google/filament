RWByteAddressBuffer prevent_dce : register(u0);

float4 dpdxFine_8c5069() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdxFine_8c5069()));
  return;
}
