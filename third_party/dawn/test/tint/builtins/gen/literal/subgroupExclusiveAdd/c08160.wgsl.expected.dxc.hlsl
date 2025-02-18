//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupExclusiveAdd_c08160() {
  int3 res = WavePrefixSum((1).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_c08160()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupExclusiveAdd_c08160() {
  int3 res = WavePrefixSum((1).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_c08160()));
  return;
}
