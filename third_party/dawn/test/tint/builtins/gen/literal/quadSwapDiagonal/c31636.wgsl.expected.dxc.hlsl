//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 quadSwapDiagonal_c31636() {
  uint4 res = QuadReadAcrossDiagonal((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_c31636()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 quadSwapDiagonal_c31636() {
  uint4 res = QuadReadAcrossDiagonal((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_c31636()));
  return;
}
