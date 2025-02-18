SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupShuffleDown_b41899() {
  int2 res = WaveReadLaneAt((1).xx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_b41899()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_b41899()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,38-55): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
