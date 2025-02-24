//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupShuffleXor_bdddba() {
  int4 res = WaveReadLaneAt((1).xxxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleXor_bdddba()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupShuffleXor_bdddba() {
  int4 res = WaveReadLaneAt((1).xxxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleXor_bdddba()));
  return;
}
