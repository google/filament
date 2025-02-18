//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> quadSwapDiagonal_af19a5() {
  vector<float16_t, 4> res = QuadReadAcrossDiagonal((float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, quadSwapDiagonal_af19a5());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> quadSwapDiagonal_af19a5() {
  vector<float16_t, 4> res = QuadReadAcrossDiagonal((float16_t(1.0h)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, quadSwapDiagonal_af19a5());
}

