SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupInclusiveAdd_7f2040() {
  vector<float16_t, 3> res = (WavePrefixSum((float16_t(1.0h)).xxx) + (float16_t(1.0h)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupInclusiveAdd_7f2040());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupInclusiveAdd_7f2040());
  return;
}
