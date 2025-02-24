SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupInclusiveMul_ac5df5() {
  vector<float16_t, 2> res = (WavePrefixProduct((float16_t(1.0h)).xx) * (float16_t(1.0h)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupInclusiveMul_ac5df5());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupInclusiveMul_ac5df5());
  return;
}
