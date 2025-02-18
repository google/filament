RWByteAddressBuffer prevent_dce : register(u0);

float2 dpdyCoarse_3e1ab4() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddy_coarse(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdyCoarse_3e1ab4()));
  return;
}
