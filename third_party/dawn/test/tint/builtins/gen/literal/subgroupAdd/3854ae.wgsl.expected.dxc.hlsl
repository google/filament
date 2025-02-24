//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupAdd_3854ae() {
  float res = WaveActiveSum(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_3854ae()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupAdd_3854ae() {
  float res = WaveActiveSum(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_3854ae()));
  return;
}
