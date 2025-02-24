RWByteAddressBuffer prevent_dce : register(u0);

float dpdxFine_f401a2() {
  float arg_0 = 1.0f;
  float res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdxFine_f401a2()));
  return;
}
