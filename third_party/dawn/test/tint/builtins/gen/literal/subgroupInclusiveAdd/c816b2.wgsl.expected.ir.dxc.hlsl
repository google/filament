//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupInclusiveAdd_c816b2() {
  int3 res = (WavePrefixSum((int(1)).xxx) + (int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_c816b2()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupInclusiveAdd_c816b2() {
  int3 res = (WavePrefixSum((int(1)).xxx) + (int(1)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_c816b2()));
}

