SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int quadSwapY_94ab6d() {
  int res = QuadReadAcrossY(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_94ab6d()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapY_94ab6d()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,13-30): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
