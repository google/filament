float tint_fwidth_fine(float v) {
  return (abs(ddx_fine(v)) + abs(ddy_fine(v)));
}

RWByteAddressBuffer prevent_dce : register(u0);

float fwidthFine_f1742d() {
  float arg_0 = 1.0f;
  float res = tint_fwidth_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidthFine_f1742d()));
  return;
}
