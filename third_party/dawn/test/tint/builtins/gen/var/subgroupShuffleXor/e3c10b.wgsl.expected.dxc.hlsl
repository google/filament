//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupShuffleXor_e3c10b() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint2 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() ^ arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_e3c10b()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupShuffleXor_e3c10b() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint2 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() ^ arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleXor_e3c10b()));
  return;
}
