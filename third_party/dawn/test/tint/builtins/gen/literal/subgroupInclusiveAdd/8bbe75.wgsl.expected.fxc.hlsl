SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupInclusiveAdd_8bbe75() {
  uint4 res = (WavePrefixSum((1u).xxxx) + (1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_8bbe75()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_8bbe75()));
  return;
}
