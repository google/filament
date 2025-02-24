SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int quadSwapDiagonal_9ccb38() {
  int arg_0 = int(1);
  int res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_9ccb38()));
}

FXC validation failure:
<scrubbed_path>(5,13-41): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
