SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupInclusiveAdd_c816b2() {
  int3 arg_0 = (1).xxx;
  int3 res = (WavePrefixSum(arg_0) + arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_c816b2()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_c816b2()));
  return;
}
