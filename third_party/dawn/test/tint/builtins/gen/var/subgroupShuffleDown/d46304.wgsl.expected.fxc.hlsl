SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffleDown_d46304() {
  uint4 arg_0 = (1u).xxxx;
  uint arg_1 = 1u;
  uint4 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() + arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_d46304()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleDown_d46304()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,38-55): error X3004: undeclared identifier 'WaveGetLaneIndex'


tint executable returned error: exit status 1
