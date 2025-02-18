SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupShuffle_aa1d5c() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  uint arg_1 = 1u;
  vector<float16_t, 2> res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffle_aa1d5c());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffle_aa1d5c());
  return;
}
FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
