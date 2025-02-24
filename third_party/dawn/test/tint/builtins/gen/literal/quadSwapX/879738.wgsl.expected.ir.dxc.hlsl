//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 quadSwapX_879738() {
  float2 res = QuadReadAcrossX((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_879738()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 quadSwapX_879738() {
  float2 res = QuadReadAcrossX((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_879738()));
}

