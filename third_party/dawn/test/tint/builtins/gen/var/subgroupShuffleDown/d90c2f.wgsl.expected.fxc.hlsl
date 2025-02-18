SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffleDown_d90c2f() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() + arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d90c2f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d90c2f()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,37-54): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
