//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 quadSwapDiagonal_a090b0() {
  int2 res = QuadReadAcrossDiagonal((1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_a090b0()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 quadSwapDiagonal_a090b0() {
  int2 res = QuadReadAcrossDiagonal((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_a090b0()));
  return;
}
