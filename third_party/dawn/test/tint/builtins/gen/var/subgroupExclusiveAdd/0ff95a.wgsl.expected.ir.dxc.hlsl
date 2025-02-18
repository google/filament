//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupExclusiveAdd_0ff95a() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupExclusiveAdd_0ff95a());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupExclusiveAdd_0ff95a() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupExclusiveAdd_0ff95a());
}

