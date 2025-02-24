//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> quadSwapDiagonal_e4bec8() {
  vector<float16_t, 3> res = QuadReadAcrossDiagonal((float16_t(1.0h)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, quadSwapDiagonal_e4bec8());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> quadSwapDiagonal_e4bec8() {
  vector<float16_t, 3> res = QuadReadAcrossDiagonal((float16_t(1.0h)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, quadSwapDiagonal_e4bec8());
}

