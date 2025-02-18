SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupInclusiveAdd_58ea3d() {
  vector<float16_t, 4> res = (WavePrefixSum((float16_t(1.0h)).xxxx) + (float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupInclusiveAdd_58ea3d());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupInclusiveAdd_58ea3d());
  return;
}
