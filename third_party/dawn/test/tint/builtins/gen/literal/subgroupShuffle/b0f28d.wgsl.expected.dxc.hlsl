//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupShuffle_b0f28d() {
  float16_t res = WaveReadLaneAt(float16_t(1.0h), 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupShuffle_b0f28d());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupShuffle_b0f28d() {
  float16_t res = WaveReadLaneAt(float16_t(1.0h), 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupShuffle_b0f28d());
  return;
}
