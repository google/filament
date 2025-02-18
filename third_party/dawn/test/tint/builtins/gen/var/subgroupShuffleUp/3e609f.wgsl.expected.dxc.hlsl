//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupShuffleUp_3e609f() {
  int4 arg_0 = (1).xxxx;
  uint arg_1 = 1u;
  int4 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_3e609f()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupShuffleUp_3e609f() {
  int4 arg_0 = (1).xxxx;
  uint arg_1 = 1u;
  int4 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffleUp_3e609f()));
  return;
}
