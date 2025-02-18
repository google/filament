
RWByteAddressBuffer prevent_dce : register(u0);
float dpdyCoarse_870a7e() {
  float arg_0 = 1.0f;
  float res = ddy_coarse(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdyCoarse_870a7e()));
}

