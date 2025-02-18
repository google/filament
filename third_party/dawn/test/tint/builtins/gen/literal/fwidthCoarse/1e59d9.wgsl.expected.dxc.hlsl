RWByteAddressBuffer prevent_dce : register(u0);

float3 fwidthCoarse_1e59d9() {
  float3 res = fwidth((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(fwidthCoarse_1e59d9()));
  return;
}
