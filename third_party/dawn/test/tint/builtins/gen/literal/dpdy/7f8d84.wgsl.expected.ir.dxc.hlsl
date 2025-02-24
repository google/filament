
RWByteAddressBuffer prevent_dce : register(u0);
float dpdy_7f8d84() {
  float res = ddy(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdy_7f8d84()));
}

