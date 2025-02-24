SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 quadSwapDiagonal_a665b1() {
  int4 res = QuadReadAcrossDiagonal((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_a665b1()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapDiagonal_a665b1()));
}

FXC validation failure:
<scrubbed_path>(4,14-50): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
