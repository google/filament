SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupShuffleUp_87c9d6() {
  float3 res = WaveReadLaneAt((1.0f).xxx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleUp_87c9d6()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleUp_87c9d6()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,44-61): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
