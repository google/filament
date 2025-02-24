RWByteAddressBuffer prevent_dce : register(u0);

float2 dpdyFine_df33aa() {
  float2 res = ddy_fine((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdyFine_df33aa()));
  return;
}
