//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 quadSwapDiagonal_a82e1d() {
  int3 arg_0 = (1).xxx;
  int3 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_a82e1d()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 quadSwapDiagonal_a82e1d() {
  int3 arg_0 = (1).xxx;
  int3 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_a82e1d()));
  return;
}
