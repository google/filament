SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupShuffleDown_c9f1c4() {
  uint2 res = WaveReadLaneAt((1u).xx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupShuffleDown_c9f1c4());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupShuffleDown_c9f1c4());
}

FXC validation failure:
<scrubbed_path>(4,40-57): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
