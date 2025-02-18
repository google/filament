SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 quadSwapDiagonal_a82e1d() {
  int3 res = QuadReadAcrossDiagonal((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_a82e1d()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_a82e1d()));
}

FXC validation failure:
<scrubbed_path>(4,14-49): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
