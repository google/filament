//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupAdd_ba53f9() {
  int res = WaveActiveSum(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_ba53f9()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupAdd_ba53f9() {
  int res = WaveActiveSum(1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_ba53f9()));
  return;
}
