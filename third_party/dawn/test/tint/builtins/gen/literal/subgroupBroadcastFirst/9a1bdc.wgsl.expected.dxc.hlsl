//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupBroadcastFirst_9a1bdc() {
  int res = WaveReadLaneFirst(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_9a1bdc()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupBroadcastFirst_9a1bdc() {
  int res = WaveReadLaneFirst(1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_9a1bdc()));
  return;
}
