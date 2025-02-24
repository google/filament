SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadSwapDiagonal_348173() {
  uint2 res = QuadReadAcrossDiagonal((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_348173()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapDiagonal_348173()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-45): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
