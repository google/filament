SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupInclusiveAdd_1b7680() {
  int arg_0 = 1;
  int res = (WavePrefixSum(arg_0) + arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_1b7680()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_1b7680()));
  return;
}
