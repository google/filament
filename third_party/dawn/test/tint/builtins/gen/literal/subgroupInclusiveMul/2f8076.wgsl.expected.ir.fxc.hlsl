SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupInclusiveMul_2f8076() {
  vector<float16_t, 3> res = (WavePrefixProduct((float16_t(1.0h)).xxx) * (float16_t(1.0h)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupInclusiveMul_2f8076());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupInclusiveMul_2f8076());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
