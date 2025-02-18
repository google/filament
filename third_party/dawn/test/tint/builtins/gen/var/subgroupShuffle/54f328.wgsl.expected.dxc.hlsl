//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffle_54f328() {
  uint arg_0 = 1u;
  int arg_1 = 1;
  uint res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffle_54f328() {
  uint arg_0 = 1u;
  int arg_1 = 1;
  uint res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}
