//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupShuffle_7c5d64() {
  float3 res = WaveReadLaneAt((1.0f).xxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_7c5d64()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupShuffle_7c5d64() {
  float3 res = WaveReadLaneAt((1.0f).xxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_7c5d64()));
}

