//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapY_0c4938() {
  uint arg_0 = 1u;
  uint res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadSwapY_0c4938());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapY_0c4938() {
  uint arg_0 = 1u;
  uint res = QuadReadAcrossY(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadSwapY_0c4938());
}

