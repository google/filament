
RWByteAddressBuffer prevent_dce : register(u0);
float3 dpdx_0763f7() {
  float3 res = ddx((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(dpdx_0763f7()));
}

