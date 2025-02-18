RWByteAddressBuffer prevent_dce : register(u0);

float fwidthCoarse_159c8a() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidthCoarse_159c8a()));
  return;
}
