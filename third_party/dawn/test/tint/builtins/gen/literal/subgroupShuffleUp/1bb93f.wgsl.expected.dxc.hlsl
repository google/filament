//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffleUp_1bb93f() {
  int res = WaveReadLaneAt(1, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_1bb93f()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffleUp_1bb93f() {
  int res = WaveReadLaneAt(1, (WaveGetLaneIndex() - 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_1bb93f()));
  return;
}
