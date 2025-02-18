//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupInclusiveAdd_e18ebb() {
  int4 res = (WavePrefixSum((1).xxxx) + (1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_e18ebb()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupInclusiveAdd_e18ebb() {
  int4 res = (WavePrefixSum((1).xxxx) + (1).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_e18ebb()));
  return;
}
