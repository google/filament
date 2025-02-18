//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupBroadcastFirst_1d9530() {
  uint2 res = WaveReadLaneFirst((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_1d9530()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupBroadcastFirst_1d9530() {
  uint2 res = WaveReadLaneFirst((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_1d9530()));
  return;
}
