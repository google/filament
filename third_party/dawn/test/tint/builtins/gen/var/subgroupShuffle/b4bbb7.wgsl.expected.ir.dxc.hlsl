//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffle_b4bbb7() {
  int3 arg_0 = (int(1)).xxx;
  uint arg_1 = 1u;
  int3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_b4bbb7()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffle_b4bbb7() {
  int3 arg_0 = (int(1)).xxx;
  uint arg_1 = 1u;
  int3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_b4bbb7()));
}

