//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupShuffleDown_c9f1c4() {
  uint2 res = WaveReadLaneAt((1u).xx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_c9f1c4()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupShuffleDown_c9f1c4() {
  uint2 res = WaveReadLaneAt((1u).xx, (WaveGetLaneIndex() + 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_c9f1c4()));
  return;
}
