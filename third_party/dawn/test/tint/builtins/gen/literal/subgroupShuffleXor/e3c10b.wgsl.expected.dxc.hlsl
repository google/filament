//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupShuffleXor_e3c10b() {
  uint2 res = WaveReadLaneAt((1u).xx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_e3c10b()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupShuffleXor_e3c10b() {
  uint2 res = WaveReadLaneAt((1u).xx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_e3c10b()));
  return;
}
