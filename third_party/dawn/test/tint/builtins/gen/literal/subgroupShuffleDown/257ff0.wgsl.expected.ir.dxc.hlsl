//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupShuffleDown_257ff0() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_257ff0()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupShuffleDown_257ff0() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, (WaveGetLaneIndex() + 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_257ff0()));
}

