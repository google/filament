//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 quadSwapDiagonal_c31636() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, quadSwapDiagonal_c31636());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 quadSwapDiagonal_c31636() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, quadSwapDiagonal_c31636());
}

