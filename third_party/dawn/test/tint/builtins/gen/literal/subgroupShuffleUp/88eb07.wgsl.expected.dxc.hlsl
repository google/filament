//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffleUp_88eb07() {
  uint4 res = WaveReadLaneAt((1u).xxxx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_88eb07()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffleUp_88eb07() {
  uint4 res = WaveReadLaneAt((1u).xxxx, (WaveGetLaneIndex() - 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_88eb07()));
  return;
}
