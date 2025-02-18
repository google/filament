//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupShuffleXor_2e033d() {
  vector<float16_t, 4> res = WaveReadLaneAt((float16_t(1.0h)).xxxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffleXor_2e033d());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupShuffleXor_2e033d() {
  vector<float16_t, 4> res = WaveReadLaneAt((float16_t(1.0h)).xxxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffleXor_2e033d());
  return;
}
