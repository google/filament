//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupExclusiveMul_13ba26() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  vector<float16_t, 3> res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupExclusiveMul_13ba26());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupExclusiveMul_13ba26() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  vector<float16_t, 3> res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupExclusiveMul_13ba26());
  return;
}
