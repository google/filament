RWByteAddressBuffer prevent_dce : register(u0);

float dpdy_7f8d84() {
  float arg_0 = 1.0f;
  float res = ddy(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdy_7f8d84()));
  return;
}
