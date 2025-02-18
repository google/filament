//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupShuffle_7ba2d5() {
  vector<float16_t, 4> res = WaveReadLaneAt((float16_t(1.0h)).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffle_7ba2d5());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupShuffle_7ba2d5() {
  vector<float16_t, 4> res = WaveReadLaneAt((float16_t(1.0h)).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffle_7ba2d5());
}

