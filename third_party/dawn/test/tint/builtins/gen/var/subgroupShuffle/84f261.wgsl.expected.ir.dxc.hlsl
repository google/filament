//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupShuffle_84f261() {
  uint4 arg_0 = (1u).xxxx;
  uint arg_1 = 1u;
  uint4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupShuffle_84f261());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupShuffle_84f261() {
  uint4 arg_0 = (1u).xxxx;
  uint arg_1 = 1u;
  uint4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupShuffle_84f261());
}

