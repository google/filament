SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupShuffleDown_313d9b() {
  int4 arg_0 = (int(1)).xxxx;
  uint arg_1 = 1u;
  int4 v = arg_0;
  uint v_1 = arg_1;
  int4 res = WaveReadLaneAt(v, (WaveGetLaneIndex() + v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_313d9b()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_313d9b()));
}

FXC validation failure:
<scrubbed_path>(8,33-50): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
