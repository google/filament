SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupInclusiveAdd_df692b() {
  float res = (WavePrefixSum(1.0f) + 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_df692b()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_df692b()));
  return;
}
