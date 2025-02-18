//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 quadSwapY_bb697b() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_bb697b()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint4 quadSwapY_bb697b() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = QuadReadAcrossY(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapY_bb697b()));
  return;
}
