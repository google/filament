SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupShuffleDown_313d9b() {
  int4 res = WaveReadLaneAt((1).xxxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_313d9b()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_313d9b()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,40-57): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
