RWByteAddressBuffer prevent_dce : register(u0);

float2 dpdx_99edb1() {
  float2 res = ddx((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(dpdx_99edb1()));
  return;
}
