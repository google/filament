//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupShuffle_647034() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  int arg_1 = 1;
  vector<float16_t, 4> res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffle_647034());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupShuffle_647034() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  int arg_1 = 1;
  vector<float16_t, 4> res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffle_647034());
  return;
}
