//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupShuffleUp_db5bcb() {
  int2 arg_0 = (1).xx;
  uint arg_1 = 1u;
  int2 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleUp_db5bcb()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupShuffleUp_db5bcb() {
  int2 arg_0 = (1).xx;
  uint arg_1 = 1u;
  int2 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleUp_db5bcb()));
  return;
}
