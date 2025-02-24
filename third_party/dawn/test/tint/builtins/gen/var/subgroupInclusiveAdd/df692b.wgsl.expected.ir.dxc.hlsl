//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupInclusiveAdd_df692b() {
  float arg_0 = 1.0f;
  float v = arg_0;
  float res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_df692b()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupInclusiveAdd_df692b() {
  float arg_0 = 1.0f;
  float v = arg_0;
  float res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_df692b()));
}

