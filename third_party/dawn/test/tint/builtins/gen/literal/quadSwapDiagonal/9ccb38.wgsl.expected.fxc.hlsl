SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int quadSwapDiagonal_9ccb38() {
  int res = QuadReadAcrossDiagonal(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,13-37): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
