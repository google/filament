
RWByteAddressBuffer prevent_dce : register(u0);
float2 dpdy_a8b56e() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddy(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdy_a8b56e()));
}

