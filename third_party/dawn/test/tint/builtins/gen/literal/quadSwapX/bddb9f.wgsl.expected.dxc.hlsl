//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadSwapX_bddb9f() {
  uint3 res = QuadReadAcrossX((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_bddb9f()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadSwapX_bddb9f() {
  uint3 res = QuadReadAcrossX((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_bddb9f()));
  return;
}
