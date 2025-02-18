SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupInclusiveAdd_f8906d() {
  float2 res = (WavePrefixSum((1.0f).xx) + (1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_f8906d()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_f8906d()));
  return;
}
