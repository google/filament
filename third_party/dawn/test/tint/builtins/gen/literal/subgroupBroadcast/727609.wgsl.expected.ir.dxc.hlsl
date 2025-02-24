//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBroadcast_727609() {
  uint4 res = WaveReadLaneAt((1u).xxxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupBroadcast_727609());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBroadcast_727609() {
  uint4 res = WaveReadLaneAt((1u).xxxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupBroadcast_727609());
}

