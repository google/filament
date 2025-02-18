SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadSwapDiagonal_856536() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_856536()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapDiagonal_856536()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-43): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
