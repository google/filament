SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupShuffleUp_3242a6() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uint res = WaveReadLaneAt(v, (WaveGetLaneIndex() - v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupShuffleUp_3242a6());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupShuffleUp_3242a6());
}

FXC validation failure:
<scrubbed_path>(8,33-50): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
