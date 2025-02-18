//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupBroadcastFirst_0e58ec() {
  vector<float16_t, 3> res = WaveReadLaneFirst((float16_t(1.0h)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupBroadcastFirst_0e58ec());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupBroadcastFirst_0e58ec() {
  vector<float16_t, 3> res = WaveReadLaneFirst((float16_t(1.0h)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupBroadcastFirst_0e58ec());
  return;
}
