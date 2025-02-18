//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> quadSwapY_a50fcb() {
  vector<float16_t, 2> res = QuadReadAcrossY((float16_t(1.0h)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, quadSwapY_a50fcb());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> quadSwapY_a50fcb() {
  vector<float16_t, 2> res = QuadReadAcrossY((float16_t(1.0h)).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, quadSwapY_a50fcb());
}

