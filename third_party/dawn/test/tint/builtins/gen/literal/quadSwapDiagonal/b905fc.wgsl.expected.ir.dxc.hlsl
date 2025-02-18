//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 quadSwapDiagonal_b905fc() {
  float3 res = QuadReadAcrossDiagonal((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_b905fc()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 quadSwapDiagonal_b905fc() {
  float3 res = QuadReadAcrossDiagonal((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_b905fc()));
}

