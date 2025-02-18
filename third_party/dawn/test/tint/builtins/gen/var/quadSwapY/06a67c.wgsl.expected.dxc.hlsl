//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadSwapY_06a67c() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_06a67c()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadSwapY_06a67c() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadAcrossY(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_06a67c()));
  return;
}
