SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupInclusiveAdd_fabbde() {
  int2 res = (WavePrefixSum((1).xx) + (1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_fabbde()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_fabbde()));
  return;
}
