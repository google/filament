//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadSwapY_a27e1c() {
  uint2 arg_0 = (1u).xx;
  uint2 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_a27e1c()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadSwapY_a27e1c() {
  uint2 arg_0 = (1u).xx;
  uint2 res = QuadReadAcrossY(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_a27e1c()));
  return;
}
