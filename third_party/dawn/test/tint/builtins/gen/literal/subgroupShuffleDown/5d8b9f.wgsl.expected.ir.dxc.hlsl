//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupShuffleDown_5d8b9f() {
  float3 res = WaveReadLaneAt((1.0f).xxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleDown_5d8b9f()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupShuffleDown_5d8b9f() {
  float3 res = WaveReadLaneAt((1.0f).xxx, (WaveGetLaneIndex() + 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleDown_5d8b9f()));
}

