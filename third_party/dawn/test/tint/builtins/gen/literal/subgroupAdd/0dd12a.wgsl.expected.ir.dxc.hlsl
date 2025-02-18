//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupAdd_0dd12a() {
  vector<float16_t, 3> res = WaveActiveSum((float16_t(1.0h)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupAdd_0dd12a());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupAdd_0dd12a() {
  vector<float16_t, 3> res = WaveActiveSum((float16_t(1.0h)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupAdd_0dd12a());
}

