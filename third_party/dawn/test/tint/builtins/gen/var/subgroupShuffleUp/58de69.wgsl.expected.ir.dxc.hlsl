//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupShuffleUp_58de69() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint2 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupShuffleUp_58de69());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupShuffleUp_58de69() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint2 res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupShuffleUp_58de69());
}

