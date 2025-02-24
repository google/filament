//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupExclusiveAdd_406ab4() {
  int4 res = WavePrefixSum((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_406ab4()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupExclusiveAdd_406ab4() {
  int4 res = WavePrefixSum((int(1)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_406ab4()));
}

