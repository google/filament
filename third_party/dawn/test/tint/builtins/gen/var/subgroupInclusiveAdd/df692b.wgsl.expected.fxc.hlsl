SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupInclusiveAdd_df692b() {
  float arg_0 = 1.0f;
  float res = (WavePrefixSum(arg_0) + arg_0);
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
