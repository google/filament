//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupInclusiveMul_4430d5() {
  vector<float16_t, 4> res = (WavePrefixProduct((float16_t(1.0h)).xxxx) * (float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupInclusiveMul_4430d5());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupInclusiveMul_4430d5() {
  vector<float16_t, 4> res = (WavePrefixProduct((float16_t(1.0h)).xxxx) * (float16_t(1.0h)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupInclusiveMul_4430d5());
  return;
}
