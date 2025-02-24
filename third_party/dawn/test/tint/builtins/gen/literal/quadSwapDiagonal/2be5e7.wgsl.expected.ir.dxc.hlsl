//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t quadSwapDiagonal_2be5e7() {
  float16_t res = QuadReadAcrossDiagonal(float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, quadSwapDiagonal_2be5e7());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t quadSwapDiagonal_2be5e7() {
  float16_t res = QuadReadAcrossDiagonal(float16_t(1.0h));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, quadSwapDiagonal_2be5e7());
}

