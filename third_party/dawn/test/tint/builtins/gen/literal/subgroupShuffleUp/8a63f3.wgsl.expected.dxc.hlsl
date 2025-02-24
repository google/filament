//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupShuffleUp_8a63f3() {
  int3 res = WaveReadLaneAt((1).xxx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleUp_8a63f3()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupShuffleUp_8a63f3() {
  int3 res = WaveReadLaneAt((1).xxx, (WaveGetLaneIndex() - 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleUp_8a63f3()));
  return;
}
