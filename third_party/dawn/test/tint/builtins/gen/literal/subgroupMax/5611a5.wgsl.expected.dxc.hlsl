//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupMax_5611a5() {
  float16_t res = WaveActiveMax(float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMax_5611a5());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupMax_5611a5() {
  float16_t res = WaveActiveMax(float16_t(1.0h));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMax_5611a5());
  return;
}
