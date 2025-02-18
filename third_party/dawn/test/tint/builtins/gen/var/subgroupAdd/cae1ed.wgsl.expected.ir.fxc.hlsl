SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupAdd_cae1ed() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupAdd_cae1ed());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupAdd_cae1ed());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
