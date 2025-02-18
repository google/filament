//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapDiagonal_9ccb38() {
  int res = QuadReadAcrossDiagonal(int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapDiagonal_9ccb38() {
  int res = QuadReadAcrossDiagonal(int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
}

