//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupInclusiveAdd_f43b30() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupInclusiveAdd_f43b30());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupInclusiveAdd_f43b30() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupInclusiveAdd_f43b30());
}

