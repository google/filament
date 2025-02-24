//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupShuffle_fb4ab9() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  uint arg_1 = 1u;
  vector<float16_t, 3> res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffle_fb4ab9());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupShuffle_fb4ab9() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  uint arg_1 = 1u;
  vector<float16_t, 3> res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffle_fb4ab9());
  return;
}
