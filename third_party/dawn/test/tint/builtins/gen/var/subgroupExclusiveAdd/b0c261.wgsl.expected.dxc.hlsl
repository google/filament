//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveAdd_b0c261() {
  int arg_0 = 1;
  int res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_b0c261()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveAdd_b0c261() {
  int arg_0 = 1;
  int res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_b0c261()));
  return;
}
