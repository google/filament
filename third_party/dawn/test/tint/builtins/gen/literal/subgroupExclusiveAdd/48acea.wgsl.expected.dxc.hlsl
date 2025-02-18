//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupExclusiveAdd_48acea() {
  uint2 res = WavePrefixSum((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_48acea()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupExclusiveAdd_48acea() {
  uint2 res = WavePrefixSum((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_48acea()));
  return;
}
