RWByteAddressBuffer prevent_dce : register(u0);

float dpdyCoarse_870a7e() {
  float res = ddy_coarse(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdyCoarse_870a7e()));
  return;
}
