RWByteAddressBuffer prevent_dce : register(u0);

float4 dpdyFine_d0a648() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddy_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdyFine_d0a648()));
  return;
}
