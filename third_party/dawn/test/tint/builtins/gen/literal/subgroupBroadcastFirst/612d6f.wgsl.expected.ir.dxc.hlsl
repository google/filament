//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBroadcastFirst_612d6f() {
  uint4 res = WaveReadLaneFirst((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupBroadcastFirst_612d6f());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBroadcastFirst_612d6f() {
  uint4 res = WaveReadLaneFirst((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupBroadcastFirst_612d6f());
}

