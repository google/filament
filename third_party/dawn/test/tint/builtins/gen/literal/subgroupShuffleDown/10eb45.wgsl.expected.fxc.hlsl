SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupShuffleDown_10eb45() {
  vector<float16_t, 4> res = WaveReadLaneAt((float16_t(1.0h)).xxxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffleDown_10eb45());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffleDown_10eb45());
  return;
}
FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
