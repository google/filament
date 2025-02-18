//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupShuffle_4f5711() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupShuffle_4f5711());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupShuffle_4f5711() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupShuffle_4f5711());
}

