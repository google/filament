//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupAdd_dcf73f() {
  float2 res = WaveActiveSum((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_dcf73f()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupAdd_dcf73f() {
  float2 res = WaveActiveSum((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_dcf73f()));
  return;
}
