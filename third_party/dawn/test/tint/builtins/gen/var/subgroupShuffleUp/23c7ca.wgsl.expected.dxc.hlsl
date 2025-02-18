//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffleUp_23c7ca() {
  float arg_0 = 1.0f;
  uint arg_1 = 1u;
  float res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_23c7ca()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffleUp_23c7ca() {
  float arg_0 = 1.0f;
  uint arg_1 = 1u;
  float res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() - arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_23c7ca()));
  return;
}
