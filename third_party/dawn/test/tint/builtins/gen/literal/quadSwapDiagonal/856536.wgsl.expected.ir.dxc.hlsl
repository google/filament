//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadSwapDiagonal_856536() {
  uint3 res = QuadReadAcrossDiagonal((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, quadSwapDiagonal_856536());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadSwapDiagonal_856536() {
  uint3 res = QuadReadAcrossDiagonal((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, quadSwapDiagonal_856536());
}

