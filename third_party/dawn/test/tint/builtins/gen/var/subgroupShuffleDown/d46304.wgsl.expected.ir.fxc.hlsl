SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupShuffleDown_d46304() {
  uint4 arg_0 = (1u).xxxx;
  uint arg_1 = 1u;
  uint4 v = arg_0;
  uint v_1 = arg_1;
  uint4 res = WaveReadLaneAt(v, (WaveGetLaneIndex() + v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupShuffleDown_d46304());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupShuffleDown_d46304());
}

FXC validation failure:
<scrubbed_path>(8,34-51): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
