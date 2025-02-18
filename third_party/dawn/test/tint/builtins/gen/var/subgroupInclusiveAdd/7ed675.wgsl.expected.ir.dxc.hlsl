//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupInclusiveAdd_7ed675() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupInclusiveAdd_7ed675());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupInclusiveAdd_7ed675() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupInclusiveAdd_7ed675());
}

