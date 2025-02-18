//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffle_030422() {
  float arg_0 = 1.0f;
  int arg_1 = 1;
  float res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_030422()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffle_030422() {
  float arg_0 = 1.0f;
  int arg_1 = 1;
  float res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_030422()));
  return;
}
