//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapX_1e1086() {
  int res = QuadReadAcrossX(int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapX_1e1086() {
  int res = QuadReadAcrossX(int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
}

