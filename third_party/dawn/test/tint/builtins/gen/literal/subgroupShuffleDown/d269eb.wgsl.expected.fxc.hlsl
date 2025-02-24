SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffleDown_d269eb() {
  int res = WaveReadLaneAt(1, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d269eb()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d269eb()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,32-49): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
