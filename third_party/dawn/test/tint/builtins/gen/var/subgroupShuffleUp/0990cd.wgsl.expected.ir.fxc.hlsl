SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupShuffleUp_0990cd() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint arg_1 = 1u;
  vector<float16_t, 4> v = arg_0;
  uint v_1 = arg_1;
  vector<float16_t, 4> res = WaveReadLaneAt(v, (WaveGetLaneIndex() - v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffleUp_0990cd());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffleUp_0990cd());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
