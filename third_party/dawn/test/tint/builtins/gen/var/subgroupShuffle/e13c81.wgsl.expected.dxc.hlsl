//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffle_e13c81() {
  uint4 arg_0 = (1u).xxxx;
  int arg_1 = 1;
  uint4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_e13c81()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupShuffle_e13c81() {
  uint4 arg_0 = (1u).xxxx;
  int arg_1 = 1;
  uint4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_e13c81()));
  return;
}
