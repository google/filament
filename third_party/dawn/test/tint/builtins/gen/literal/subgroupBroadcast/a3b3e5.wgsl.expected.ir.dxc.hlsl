//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupBroadcast_a3b3e5() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupBroadcast_a3b3e5());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupBroadcast_a3b3e5() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupBroadcast_a3b3e5());
}

