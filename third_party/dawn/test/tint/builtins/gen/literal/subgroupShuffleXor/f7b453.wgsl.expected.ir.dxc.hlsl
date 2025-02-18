//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupShuffleXor_f7b453() {
  float2 res = WaveReadLaneAt((1.0f).xx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_f7b453()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupShuffleXor_f7b453() {
  float2 res = WaveReadLaneAt((1.0f).xx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_f7b453()));
}

