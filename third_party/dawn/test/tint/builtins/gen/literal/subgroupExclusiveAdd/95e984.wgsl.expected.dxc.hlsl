//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupExclusiveAdd_95e984() {
  vector<float16_t, 4> res = WavePrefixSum((float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupExclusiveAdd_95e984());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupExclusiveAdd_95e984() {
  vector<float16_t, 4> res = WavePrefixSum((float16_t(1.0h)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupExclusiveAdd_95e984());
  return;
}
