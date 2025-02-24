//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupAdd_b61df7() {
  uint arg_0 = 1u;
  uint res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_b61df7()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupAdd_b61df7() {
  uint arg_0 = 1u;
  uint res = WaveActiveSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_b61df7()));
  return;
}
