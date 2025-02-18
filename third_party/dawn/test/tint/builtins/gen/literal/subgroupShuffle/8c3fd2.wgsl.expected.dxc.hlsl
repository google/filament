//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupShuffle_8c3fd2() {
  vector<float16_t, 2> res = WaveReadLaneAt((float16_t(1.0h)).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffle_8c3fd2());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupShuffle_8c3fd2() {
  vector<float16_t, 2> res = WaveReadLaneAt((float16_t(1.0h)).xx, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffle_8c3fd2());
  return;
}
