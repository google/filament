//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadSwapDiagonal_348173() {
  uint2 res = QuadReadAcrossDiagonal((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_348173()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadSwapDiagonal_348173() {
  uint2 res = QuadReadAcrossDiagonal((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_348173()));
  return;
}
