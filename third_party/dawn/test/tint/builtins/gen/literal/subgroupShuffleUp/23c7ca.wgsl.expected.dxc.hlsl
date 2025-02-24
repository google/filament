//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffleUp_23c7ca() {
  float res = WaveReadLaneAt(1.0f, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_23c7ca()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffleUp_23c7ca() {
  float res = WaveReadLaneAt(1.0f, (WaveGetLaneIndex() - 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_23c7ca()));
  return;
}
