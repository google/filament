//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupInclusiveAdd_1b7680() {
  int arg_0 = int(1);
  int v = arg_0;
  int res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_1b7680()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupInclusiveAdd_1b7680() {
  int arg_0 = int(1);
  int v = arg_0;
  int res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_1b7680()));
}

