
RWByteAddressBuffer prevent_dce : register(u0);
float fwidthFine_f1742d() {
  float arg_0 = 1.0f;
  float v = arg_0;
  float v_1 = ddx_fine(v);
  float res = (abs(v_1) + abs(ddy_fine(v)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidthFine_f1742d()));
}

