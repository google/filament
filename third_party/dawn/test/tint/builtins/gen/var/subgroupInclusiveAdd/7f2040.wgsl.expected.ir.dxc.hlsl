//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupInclusiveAdd_7f2040() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  vector<float16_t, 3> v = arg_0;
  vector<float16_t, 3> res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupInclusiveAdd_7f2040());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupInclusiveAdd_7f2040() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  vector<float16_t, 3> v = arg_0;
  vector<float16_t, 3> res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupInclusiveAdd_7f2040());
}

