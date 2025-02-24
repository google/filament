
RWByteAddressBuffer prevent_dce : register(u0);
float3 dpdxCoarse_f64d7b() {
  float3 res = ddx_coarse((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(dpdxCoarse_f64d7b()));
}

