//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 quadSwapDiagonal_8077c8() {
  float2 res = QuadReadAcrossDiagonal((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_8077c8()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 quadSwapDiagonal_8077c8() {
  float2 res = QuadReadAcrossDiagonal((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_8077c8()));
  return;
}
