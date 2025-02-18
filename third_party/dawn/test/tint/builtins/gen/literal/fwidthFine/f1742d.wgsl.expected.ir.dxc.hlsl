
RWByteAddressBuffer prevent_dce : register(u0);
float fwidthFine_f1742d() {
  float v = ddx_fine(1.0f);
  float res = (abs(v) + abs(ddy_fine(1.0f)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidthFine_f1742d()));
}

