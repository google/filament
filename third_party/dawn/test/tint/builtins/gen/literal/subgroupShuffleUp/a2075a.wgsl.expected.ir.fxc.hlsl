SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupShuffleUp_a2075a() {
  vector<float16_t, 2> res = WaveReadLaneAt((float16_t(1.0h)).xx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffleUp_a2075a());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffleUp_a2075a());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
