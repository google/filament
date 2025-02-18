//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapDiagonal_331804() {
  float4 res = QuadReadAcrossDiagonal((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_331804()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapDiagonal_331804() {
  float4 res = QuadReadAcrossDiagonal((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_331804()));
}

