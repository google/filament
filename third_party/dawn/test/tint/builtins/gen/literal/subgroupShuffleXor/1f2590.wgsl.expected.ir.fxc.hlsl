SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupShuffleXor_1f2590() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleXor_1f2590());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleXor_1f2590());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
