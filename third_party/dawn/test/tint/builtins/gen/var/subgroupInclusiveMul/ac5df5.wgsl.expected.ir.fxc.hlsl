SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupInclusiveMul_ac5df5() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> v = arg_0;
  vector<float16_t, 2> res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupInclusiveMul_ac5df5());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupInclusiveMul_ac5df5());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
