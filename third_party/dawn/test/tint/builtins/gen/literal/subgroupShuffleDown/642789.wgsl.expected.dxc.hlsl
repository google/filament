//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupShuffleDown_642789() {
  uint3 res = WaveReadLaneAt((1u).xxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleDown_642789()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupShuffleDown_642789() {
  uint3 res = WaveReadLaneAt((1u).xxx, (WaveGetLaneIndex() + 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleDown_642789()));
  return;
}
