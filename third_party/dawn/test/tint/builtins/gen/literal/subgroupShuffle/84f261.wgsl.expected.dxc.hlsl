//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffle_84f261() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_84f261()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffle_84f261() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_84f261()));
  return;
}
