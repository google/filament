//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapDiagonal_730e40() {
  uint res = QuadReadAcrossDiagonal(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadSwapDiagonal_730e40());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapDiagonal_730e40() {
  uint res = QuadReadAcrossDiagonal(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadSwapDiagonal_730e40());
}

