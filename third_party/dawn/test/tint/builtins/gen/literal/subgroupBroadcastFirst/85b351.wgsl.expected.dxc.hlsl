//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupBroadcastFirst_85b351() {
  int2 res = WaveReadLaneFirst((1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_85b351()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupBroadcastFirst_85b351() {
  int2 res = WaveReadLaneFirst((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_85b351()));
  return;
}
