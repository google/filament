
RWByteAddressBuffer prevent_dce : register(u0);
float dpdyFine_6eb673() {
  float arg_0 = 1.0f;
  float res = ddy_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdyFine_6eb673()));
}

