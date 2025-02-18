
RWByteAddressBuffer prevent_dce : register(u0);
float dpdx_e263de() {
  float res = ddx(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdx_e263de()));
}

