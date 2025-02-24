float2 tint_fwidth_fine(float2 v) {
  return (abs(ddx_fine(v)) + abs(ddy_fine(v)));
}

RWByteAddressBuffer prevent_dce : register(u0);

float2 fwidthFine_ff6aa0() {
  float2 arg_0 = (1.0f).xx;
  float2 res = tint_fwidth_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(fwidthFine_ff6aa0()));
  return;
}
