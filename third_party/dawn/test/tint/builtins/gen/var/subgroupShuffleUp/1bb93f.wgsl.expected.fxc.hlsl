SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffleUp_1bb93f() {
  int arg_0 = 1;
  uint arg_1 = 1u;
  int res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_1bb93f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_1bb93f()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,36-53): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
