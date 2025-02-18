//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcast_49de94() {
  uint res = WaveReadLaneAt(1u, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupBroadcast_49de94());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcast_49de94() {
  uint res = WaveReadLaneAt(1u, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupBroadcast_49de94());
}

