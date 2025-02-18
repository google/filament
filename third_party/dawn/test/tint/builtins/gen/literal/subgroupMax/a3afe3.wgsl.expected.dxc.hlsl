//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupMax_a3afe3() {
  vector<float16_t, 2> res = WaveActiveMax((float16_t(1.0h)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupMax_a3afe3());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupMax_a3afe3() {
  vector<float16_t, 2> res = WaveActiveMax((float16_t(1.0h)).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupMax_a3afe3());
  return;
}
