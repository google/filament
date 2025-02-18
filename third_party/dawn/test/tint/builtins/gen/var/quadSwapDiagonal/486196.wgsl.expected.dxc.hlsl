//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float quadSwapDiagonal_486196() {
  float arg_0 = 1.0f;
  float res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_486196()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float quadSwapDiagonal_486196() {
  float arg_0 = 1.0f;
  float res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_486196()));
  return;
}
