//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapY_b9d9e7() {
  float4 res = QuadReadAcrossY((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_b9d9e7()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadSwapY_b9d9e7() {
  float4 res = QuadReadAcrossY((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_b9d9e7()));
}

