//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupBroadcast_13f36c() {
  vector<float16_t, 2> res = WaveReadLaneAt((float16_t(1.0h)).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupBroadcast_13f36c());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupBroadcast_13f36c() {
  vector<float16_t, 2> res = WaveReadLaneAt((float16_t(1.0h)).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupBroadcast_13f36c());
}

