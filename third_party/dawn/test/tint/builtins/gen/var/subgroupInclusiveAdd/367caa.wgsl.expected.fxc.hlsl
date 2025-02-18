SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupInclusiveAdd_367caa() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = (WavePrefixSum(arg_0) + arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_367caa()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_367caa()));
  return;
}
