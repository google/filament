//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffle_8bfbcd() {
  int arg_0 = 1;
  int arg_1 = 1;
  int res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_8bfbcd()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffle_8bfbcd() {
  int arg_0 = 1;
  int arg_1 = 1;
  int res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_8bfbcd()));
  return;
}
