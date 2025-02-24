//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupInclusiveAdd_f43b30() {
  uint3 res = (WavePrefixSum((1u).xxx) + (1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_f43b30()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupInclusiveAdd_f43b30() {
  uint3 res = (WavePrefixSum((1u).xxx) + (1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_f43b30()));
  return;
}
