RWByteAddressBuffer prevent_dce : register(u0);

float dpdxCoarse_029152() {
  float res = ddx_coarse(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdxCoarse_029152()));
  return;
}
