//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapX_8203ad() {
  uint res = QuadReadAcrossX(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadSwapX_8203ad());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapX_8203ad() {
  uint res = QuadReadAcrossX(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadSwapX_8203ad());
}

