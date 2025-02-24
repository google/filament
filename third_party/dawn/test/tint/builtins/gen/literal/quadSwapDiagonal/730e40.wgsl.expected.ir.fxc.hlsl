SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint quadSwapDiagonal_730e40() {
  uint res = QuadReadAcrossDiagonal(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadSwapDiagonal_730e40());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadSwapDiagonal_730e40());
}

FXC validation failure:
<scrubbed_path>(4,14-39): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
