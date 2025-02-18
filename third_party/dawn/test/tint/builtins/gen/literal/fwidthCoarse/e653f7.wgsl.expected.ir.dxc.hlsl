
RWByteAddressBuffer prevent_dce : register(u0);
float2 fwidthCoarse_e653f7() {
  float2 res = fwidth((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(fwidthCoarse_e653f7()));
}

