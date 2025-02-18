SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 quadSwapX_07f1fc() {
  uint4 res = QuadReadAcrossX((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_07f1fc()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadSwapX_07f1fc()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-40): error X3004: undeclared identifier 'QuadReadAcrossX'


tint executable returned error: exit status 1
