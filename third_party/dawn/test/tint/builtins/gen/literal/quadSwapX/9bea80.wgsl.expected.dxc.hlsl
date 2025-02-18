//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float quadSwapX_9bea80() {
  float res = QuadReadAcrossX(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_9bea80()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float quadSwapX_9bea80() {
  float res = QuadReadAcrossX(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_9bea80()));
  return;
}
