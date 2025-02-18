//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapX_b1a5fe() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_b1a5fe()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapX_b1a5fe() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadAcrossX(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapX_b1a5fe()));
}

