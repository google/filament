SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float quadSwapDiagonal_486196() {
  float arg_0 = 1.0f;
  float res = QuadReadAcrossDiagonal(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_486196()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapDiagonal_486196()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-43): error X3004: undeclared identifier 'QuadReadAcrossDiagonal'


tint executable returned error: exit status 1
