
RWByteAddressBuffer prevent_dce : register(u0);
float4 dpdx_c487fa() {
  float4 res = ddx((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdx_c487fa()));
}

