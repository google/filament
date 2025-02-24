float4 tint_fwidth_fine(float4 v) {
  return (abs(ddx_fine(v)) + abs(ddy_fine(v)));
}

RWByteAddressBuffer prevent_dce : register(u0);

float4 fwidthFine_68f4ef() {
  float4 res = tint_fwidth_fine((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(fwidthFine_68f4ef()));
  return;
}
