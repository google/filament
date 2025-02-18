SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupShuffleUp_3e609f() {
  int4 res = WaveReadLaneAt((int(1)).xxxx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_3e609f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_3e609f()));
}

FXC validation failure:
<scrubbed_path>(4,45-62): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
