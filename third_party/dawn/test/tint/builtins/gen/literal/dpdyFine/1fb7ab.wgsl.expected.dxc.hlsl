RWByteAddressBuffer prevent_dce : register(u0);

float3 dpdyFine_1fb7ab() {
  float3 res = ddy_fine((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(dpdyFine_1fb7ab()));
  return;
}
