//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> quadSwapX_d60cec() {
  vector<float16_t, 2> res = QuadReadAcrossX((float16_t(1.0h)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, quadSwapX_d60cec());
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> quadSwapX_d60cec() {
  vector<float16_t, 2> res = QuadReadAcrossX((float16_t(1.0h)).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, quadSwapX_d60cec());
  return;
}
