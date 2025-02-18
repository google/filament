//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcast_c36fe1() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupBroadcast_c36fe1());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcast_c36fe1() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupBroadcast_c36fe1());
}

