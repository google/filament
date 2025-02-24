SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupInclusiveAdd_9bbcb0() {
  uint2 arg_0 = (1u).xx;
  uint2 res = (WavePrefixSum(arg_0) + arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_9bbcb0()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_9bbcb0()));
  return;
}
