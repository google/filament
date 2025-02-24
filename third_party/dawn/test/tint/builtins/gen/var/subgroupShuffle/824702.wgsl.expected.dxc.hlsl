//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupShuffle_824702() {
  int3 arg_0 = (1).xxx;
  int arg_1 = 1;
  int3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_824702()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupShuffle_824702() {
  int3 arg_0 = (1).xxx;
  int arg_1 = 1;
  int3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_824702()));
  return;
}
