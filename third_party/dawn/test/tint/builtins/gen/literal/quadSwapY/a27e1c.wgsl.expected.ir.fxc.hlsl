SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadSwapY_a27e1c() {
  uint2 res = QuadReadAcrossY((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadSwapY_a27e1c());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadSwapY_a27e1c());
}

FXC validation failure:
<scrubbed_path>(4,15-38): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
