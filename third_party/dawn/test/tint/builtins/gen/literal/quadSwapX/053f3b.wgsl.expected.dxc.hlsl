//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 quadSwapX_053f3b() {
  int3 res = QuadReadAcrossX((1).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_053f3b()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 quadSwapX_053f3b() {
  int3 res = QuadReadAcrossX((1).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapX_053f3b()));
  return;
}
