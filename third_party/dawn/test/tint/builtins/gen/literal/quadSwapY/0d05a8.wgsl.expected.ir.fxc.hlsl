SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int2 quadSwapY_0d05a8() {
  int2 res = QuadReadAcrossY((int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_0d05a8()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadSwapY_0d05a8()));
}

FXC validation failure:
<scrubbed_path>(4,14-41): error X3004: undeclared identifier 'QuadReadAcrossY'


tint executable returned error: exit status 1
