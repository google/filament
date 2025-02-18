//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float4 quadSwapY_b9d9e7() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_b9d9e7()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float4 quadSwapY_b9d9e7() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadAcrossY(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_b9d9e7()));
  return;
}
