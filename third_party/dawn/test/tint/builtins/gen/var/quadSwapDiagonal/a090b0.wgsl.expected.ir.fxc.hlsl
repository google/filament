SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapDiagonal_a090b0() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_a090b0()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_a090b0()));
}

FXC validation failure:
<scrubbed_path>(5,14-42): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
