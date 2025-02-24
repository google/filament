//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMax_b58cbf() {
  uint res = WaveActiveMax(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_b58cbf()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMax_b58cbf() {
  uint res = WaveActiveMax(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_b58cbf()));
  return;
}
