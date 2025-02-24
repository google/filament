//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupExclusiveAdd_0ff95a() {
  uint3 res = WavePrefixSum((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_0ff95a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupExclusiveAdd_0ff95a() {
  uint3 res = WavePrefixSum((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_0ff95a()));
  return;
}
