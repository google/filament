//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupInclusiveAdd_a7c60f() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> v = arg_0;
  vector<float16_t, 2> res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupInclusiveAdd_a7c60f());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupInclusiveAdd_a7c60f() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> v = arg_0;
  vector<float16_t, 2> res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupInclusiveAdd_a7c60f());
}

