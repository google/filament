SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffleUp_3242a6() {
  uint res = WaveReadLaneAt(1u, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_3242a6()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_3242a6()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,34-51): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
