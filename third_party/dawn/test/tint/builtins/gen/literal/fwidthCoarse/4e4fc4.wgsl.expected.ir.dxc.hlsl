
RWByteAddressBuffer prevent_dce : register(u0);
float4 fwidthCoarse_4e4fc4() {
  float4 res = fwidth((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(fwidthCoarse_4e4fc4()));
}

