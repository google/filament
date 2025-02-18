//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupShuffleUp_db5bcb() {
  int2 res = WaveReadLaneAt((int(1)).xx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleUp_db5bcb()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupShuffleUp_db5bcb() {
  int2 res = WaveReadLaneAt((int(1)).xx, (WaveGetLaneIndex() - 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleUp_db5bcb()));
}

