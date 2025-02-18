//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupBroadcast_9ccdca() {
  int res = WaveReadLaneAt(1, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_9ccdca()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupBroadcast_9ccdca() {
  int res = WaveReadLaneAt(1, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_9ccdca()));
  return;
}
